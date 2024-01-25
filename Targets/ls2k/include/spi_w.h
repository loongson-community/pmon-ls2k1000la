void spi_jedec_id(void);
int selected_lpc_spi(void* base);
int spi_fl_erase_device(void *fl_base, int size, int verbose);
int spi_fl_program_device(void *fl_base, void *data_base, int data_size, int verbose);
struct fl_device *spi_fl_devident(void *base, struct fl_map **m);
