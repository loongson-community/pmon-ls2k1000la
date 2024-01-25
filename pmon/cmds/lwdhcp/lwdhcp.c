/////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                 //	
//                                  Light weight DHCP client                                       //
//                                                                                                 //
/////////////////////////////////////////////////////////////////////////////////////////////////////


#include <sys/param.h>
#include <sys/file.h>
#include <sys/syslog.h>
#include <sys/endian.h>

#include "packet.h"
#ifdef		PMON
	#include <sys/ioctl.h>
	#define	 KERNEL
	#include <pmon/netio/bootp.h>
	#include <sys/types.h>
#else
	#include <linux/if_packet.h>
	#include <linux/if_ether.h>
	#include <net/if.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
#endif

#include <net/route.h>
#include <pmon.h>
#include <setjmp.h>
#include <signal.h>

#include "lwdhcp.h"
#include "options.h"
extern int errno;

struct client_config_t  client_config;
int 	dhcp_request;
int		fd = 0;
sig_t	pre_handler = 0;
static struct ifreq ifr;

static jmp_buf  jmpb;

static void terminate()
{
	DbgPrint("Program terminated by user.\n");
	dhcp_request = 0;
	
	if(fd > 0)
		close(fd);
	
	longjmp(jmpb, 1);
}

static void	init(char *ethname)
{
	memset((void *)&client_config, 0, sizeof(struct client_config_t));

	strcpy(client_config.interface, ethname);

	pre_handler = signal(SIGINT, (sig_t)terminate);
}

int		listen_socket()
{
	int		sock, n;
	int		flag;
	int		dwValue;
	struct sockaddr_in		clnt;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
	{
		PERROR("socket create error");
		return sock;
	}

	bzero((char*)&clnt, sizeof(clnt));
	clnt.sin_family = AF_INET;
	clnt.sin_port = htons(CLIENT_PORT);
	clnt.sin_addr.s_addr = INADDR_ANY;

	if (bind (sock, (struct sockaddr *)&clnt, sizeof(struct sockaddr_in)) < 0) {
		PERROR ("bind failed");
		close (sock);
		return -1;
	}

	flag = 1;
	setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char*)&flag, sizeof(flag)); 

	n = 1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&n, sizeof(n));

	return sock;
}


int read_interface(char *interface, int *ifindex, uint32_t *addr, uint8_t *arp)
{
	int fd;
	struct sockaddr_in *our_ip;

	memset(&ifr, 0, sizeof(struct ifreq));
	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) 
	{
		ifr.ifr_addr.sa_family = AF_INET;

		strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);

		if (addr) 
		{
			if (ioctl(fd, SIOCGIFADDR, &ifr) != 0) 
			{
				struct sockaddr_in *sa =  (void *)&ifr.ifr_addr;
				(void) ioctl(fd,SIOCGIFFLAGS,&ifr);
				ifr.ifr_flags |=IFF_UP;
				(void) ioctl(fd,SIOCSIFFLAGS,&ifr);
    				bzero (sa, sizeof (*sa));
				sa->sin_len = sizeof (*sa);
				sa->sin_family = AF_INET;
				sa->sin_addr.s_addr = inet_addr("0.0.0.0");
				(void) ioctl(fd, SIOCSIFADDR, &ifr);
			}
			if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) 
			{
				our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
				*addr = our_ip->sin_addr.s_addr;
				DbgPrint("%s (our ip) = %s\n", ifr.ifr_name, inet_ntoa(our_ip->sin_addr));
			} else 
			{
				PERROR("SIOCGIFADDR failed, is the interface up and configured?\n");
				close(fd);
				return -1;
			}
		}

		//the following code needs to change the PMON source code pmon/pmon/netio/bootp.c line 101
		//int
		//getethaddr (unsigned char *eaddr, char *ifc)
		//{
		//    struct ifnet *ifp;
		//        struct ifaddr *ifa;
		// ......
#ifndef		PMON
		if(ioctl(fd, SIOCGIFINDEX, &ifr) == 0) 
		{
			DbgPrint("adapter index %d\n", ifr.ifr_ifindex);
			*ifindex = ifr.ifr_ifindex;
		} else 
		{
			PERROR("SIOCGIFINDEX failed!\n");
			close(fd);
			return -1;
		}
#endif

#ifndef		PMON
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) 
		{
			memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
#else
		if(getethaddr(arp, client_config.interface) >= 0)
		{
#endif
			DbgPrint("adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x\n",
					arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
		} 
		else{
			PERROR("SIOCGIFHWADDR failed!\n");
			close(fd);
			return -1;
		}
	} 
	else {
		PERROR("socket failed!\n");
		return -1;
	}
	close(fd);
	return 0;
}

static void
setsin (struct sockaddr_in *sa, int family, u_long addr)
{
    bzero (sa, sizeof (*sa));
    sa->sin_len = sizeof (*sa);
    sa->sin_family = family;
    sa->sin_addr.s_addr = addr;
}


#define SIN(x) ((struct sockaddr_in *)&(x))
#define SAD(x) ((struct sockaddr *)&(x))
int lwdhcp(int argc, char* argv[])
{
	int						xid;
	fd_set					fs;
	int						ret, err_no, totimes;
	char					buf[1500];
	struct sockaddr_in		from, *sa;
	struct sockaddr zmask;
	int						size = sizeof(from);
	struct dhcp_packet*		p;
	uint8_t*				dhcp_message_type;
	struct	timeval			tv;
    unsigned int            mask;
    struct ifaliasreq ifra;

	if(getuid())
	{
		DbgPrint("Only root can run this program!\n");
		return 0;
	}

	DbgPrint("Light weight DHCP client starts...\n");
	init(argc>1?argv[1]:"syn0");

	if(setjmp(jmpb))
	{
		signal(SIGINT, pre_handler);
		sigsetmask(0);
		return 0;
	}

	
	if(read_interface(client_config.interface, &client_config.ifindex,
				&client_config.addr, client_config.arp) < 0)
	{
		DbgPrint("read_interface error");
		return 0;
	}

	if((fd = listen_socket()) < 0)
	{
		return 0;
	}

	//srand(time(NULL));
	//xid = rand();
	totimes = 10;

tryagain:
    printf("fsy::xid=%x\n");
	if(send_discover(xid) < 0)
		if(--totimes > 0)
			goto tryagain;
		else
		{
			DbgPrint("Fail to send DHCPDISCOVER...\n");
			return 0;
		}

	FD_ZERO(&fs);
	FD_SET(fd, &fs);
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	//receiving DHCPOFFER
	while(1)
	{
		dhcp_request = 1;
		ret = select(fd + 1, &fs, NULL, NULL, &tv);
		
		if(ret == -1)
			PERROR("select error");
		else if(ret == 0)
		{
            DbgPrint("select timeout");
			if(--totimes > 0)
				goto tryagain;
			else
			{
				dhcp_request = 0;
				DbgPrint("Fail to get IP from DHCP server, it seems that there is no DHCP server.\n");
				close(fd);
				return 0;
			}
		}

		size = sizeof(from);
		if(recvfrom(fd, buf, sizeof(struct dhcp_packet), (struct sockaddr *)0, &from, &size) < 0)
			continue;
		
		dhcp_request = 0;

		p = (struct dhcp_packet *)buf;

		if(p->xid != xid)
			continue;

		dhcp_message_type = get_dhcp_option(p, DHCP_MESSAGE_TYPE);
		if(!dhcp_message_type)
			continue;
		else if(*dhcp_message_type != DHCPOFFER)
			continue;

		DbgPrint("DHCPOFFER received...\n");
		break;
	}

	//sending DHCPREQUEST
	DbgPrint("(from ip) = %s\n", inet_ntoa(from.sin_addr));
	DbgPrint("(yi ip) = %s\n", inet_ntoa(p->yiaddr));
	send_request(xid, *((uint32_t*)(&from.sin_addr)), *((uint32_t*)(&p->yiaddr)));

	tv.tv_sec = 3;
	tv.tv_usec = 0;
	//receiving DHCPACK
	while(1)
	{
		dhcp_request = 1;
		ret = select(fd + 1, &fs, NULL, NULL, &tv);
		
		if(ret == -1)
			PERROR("select error");
		else if(ret == 0)
		{
            DbgPrint("select timeout");
			if(--totimes > 0)
				goto tryagain;
			else
			{
				dhcp_request = 0;
				DbgPrint("Fail to get IP from DHCP server, no ACK from DHCP server.\n");
				close(fd);
				return 0;
			}
		}

		//get_raw_packet(buf, fd);
		
		size = sizeof(struct sockaddr);
		recvfrom(fd, buf, sizeof(struct dhcp_packet), 0, (struct sockaddr*)&from, &size);

		dhcp_request = 0;
		
		if(p->xid != xid)
			continue;

		dhcp_message_type = get_dhcp_option(p, DHCP_MESSAGE_TYPE);
		if(!dhcp_message_type)
			continue;
		else if(*dhcp_message_type != DHCPACK)
			continue;

		DbgPrint("DHCPACK received...\n");
		DbgPrint("IP %s obtained from the DHCP server.\n", inet_ntoa(p->yiaddr));

        bzero (&ifra, sizeof(ifra));
        sa = (void *)&ifra.ifra_mask;
        strcpy(ifra.ifra_name, argc>1?argv[1]:"syn0");
        mask = 0;
        while((p->options[mask] != 1)) {
            mask += (p->options[mask + 1] + 2);
            if(p->options[mask] == 0xff){
                DbgPrint("dhcp pack options no netmask\n");
                break;
            }
            if(mask >= (DHCP_OPTION_LEN - 4)){
                DbgPrint("dhcp pack options array bound\n");
                break;
            }
        }
        if (p->options[mask] == 1){
            mask += 2;
            sa->sin_addr.s_addr = (p->options[mask + 3] << 24) | (p->options[mask + 2]<< 16) | (p->options[mask + 1] << 8) | (p->options[mask]);

        }   
        else{
            sa->sin_addr.s_addr = 0x00ffffff;
            DbgPrint("No netmask, set 255.255.255.0\n");
        }
        setsin (SIN(ifra.ifra_addr), AF_INET, p->yiaddr.s_addr);
        setsin (SIN(ifra.ifra_mask), 0, sa->sin_addr.s_addr);
        (void) ioctl(fd, SIOCSIFADDR, &ifra);
        if (ioctl(fd, SIOCAIFADDR, &ifra) < 0) {
            printf ( "\nNOTICE: No network interface available\n");
            close (fd);
            return 0;
        }
        
        struct sockaddr dst;
        struct sockaddr gate;
	    bzero (&zmask, sizeof(zmask));
        setsin (SIN(dst), AF_INET, INADDR_ANY);
        setsin (SIN(gate), AF_INET, p->giaddr.s_addr);
		if (SIN(gate)->sin_addr.s_addr != (u_long)-1) {
			err_no = rtrequest (RTM_ADD, &dst, &gate, &zmask,
			       RTF_UP | RTF_GATEWAY, (struct rtentry **)0);
			if (err_no) {
				DbgPrint("route add gateway");
			}
		} else {
			DbgPrint ("gateway: unknown host\n");
		}
		break;
	}
    
    errno = 0;
	close(fd);
	signal(SIGINT, pre_handler);
	sigsetmask(0);

	return 0;
}



/*
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Network"},
	{"lwdhcp",    "", 0,
		"Light weight DHCP client",
		lwdhcp, 1, 99, CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

	static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}





