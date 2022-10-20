/* date = October 16th 2021 9:15 pm */

#ifndef MOMO_CRC_H
#define MOMO_CRC_H


//~ NOTE(Momo): 'Slow' series
// These are for quick and easy 
static inline u32
CRC32_Slow(u8* data, u32 data_size, u32 start_register, u32 polynomial) {
	u32 r = start_register;
	for (u32 i = 0; i < data_size; ++i ){
		r ^= data[i] <<  24;
		for (u32 j = 0; j < 8; ++j) {
			if((r & 0x80000000) != 0) {
				r = (u32)((r << 1) ^ polynomial);
			}
			else {
				r <<= 1;
			}
		}
	}
	return r;
}


static inline u32
CRC16_Slow(u8* data, u32 data_size, u16 start_register, u16 polynomial) {
	u32 r = start_register;
	for (u32 i = 0; i < data_size; ++i ){
		r ^= data[i] << 8;
		for (u32 j = 0; j < 8; ++j) {
			if((r & 0x8000) != 0) {
				r = (u32)((r << 1) ^ polynomial);
			}
			else {
				r <<= 1;
			}
		}
	}
	return r;
}

static inline u32
CRC8_Slow(u8* data, u32 data_size, u16 start_register, u16 polynomial) {
	u32 r = start_register;
	for (u32 i = 0; i < data_size; ++i ){
		r ^= data[i];
		for (u32 j = 0; j < 8; ++j) {
			if((r & 0x80) != 0) {
				r = (u32)((r << 1) ^ polynomial);
			}
			else {
				r <<= 1;
			}
		}
	}
	return r;
}

//~ NOTE(Momo): Table generation 
struct CRC32_Table {
	u32 remainders[256];
};

struct CRC16_Table {
	u16 remainders[256];
};

struct CRC8_Table {
	u8 remainders[256];
};

static inline CRC32_Table
CRC32_GenerateTable(u32 polynomial) {
	CRC32_Table table = {};
	for (u32 divident = 0; divident < 256; ++divident) {
		u32 remainder = divident <<  24;
		for (u32 j = 0; j < 8; ++j) {
			if((remainder & 0x80000000) != 0) {
				remainder = (u32)((remainder << 1) ^ polynomial);
			}
			else {
				remainder <<= 1;
			}
		}
		table.remainders[divident] = remainder;
	}
	return table;
}

static inline CRC16_Table
CRC16_GenerateTable(u16 polynomial) {
	CRC16_Table table = {};
	for (u16 divident = 0; divident < 256; ++divident) {
		u16 remainder = divident << 8;
		for (u32 j = 0; j < 8; ++j) {
			if((remainder & 0x8000) != 0) {
				remainder = (u16)((remainder << 1) ^ polynomial);
			}
			else {
				remainder <<= 1;
			}
		}
		table.remainders[divident] = remainder;
	}
	return table;
}

static inline CRC8_Table
CRC8_GenerateTable(u8 polynomial) {
	CRC8_Table table = {};
	for (u8 divident = 0; divident < 256; ++divident) {
		u8 remainder = divident;
		for (u32 j = 0; j < 8; ++j) {
			if((remainder & 0x80) != 0) {
				remainder = (u8)((remainder << 1) ^ polynomial);
			}
			else {
				remainder <<= 1;
			}
		}
		table.remainders[divident] = remainder;
	}
	return table;
}

//~ NOTE(Momo): Fast series. Uses a table
static inline u32
CRC32(u8* data, u32 data_size, u16 start_register, CRC32_Table table) {
	u32 crc = start_register;
	for (u32 i = 0; i < data_size; ++i) {
		u32 divident = (u32)((crc ^ (data[i] << 24)) >> 24);
		crc = (u32)((crc << 8) ^ (u32)(table.remainders[divident]));
	}
	return crc;
}

static inline u32
CRC16(u8* data, u32 data_size, u16 start_register, CRC16_Table table) {
	u16 crc = start_register;
	for (u32 i = 0; i < data_size; ++i) {
		u16 divident = (u16)((crc ^ (data[i] << 8)) >> 8);
		crc = (u16)((crc << 8) ^ (u16)(table.remainders[divident]));
	}
	return crc;
}

static inline u32
CRC8(u8* data, u32 data_size, u8 start_register, CRC8_Table table) {
	u8 crc = start_register;
	for (u32 i = 0; i < data_size; ++i) {
		u8 divident = (u8)(crc ^ data[i]);
		crc = table.remainders[divident];
	}
	return crc;
}




#endif //MOMO_CRC_H
