typedef struct
{
  uint8_t       download_ok;
  int32_t       program_size;
  uint32_t      available_size;
  char          drive[2];
  uint8_t       update_program;
	uint8_t				not_enough_size;
} FOTA_STRUC;

extern FOTA_STRUC fota;

void FOTA_Serv(void);


