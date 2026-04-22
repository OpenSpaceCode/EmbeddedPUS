#include "cunit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/pus.h"
#include "../include/pus_config.h"
#include "../external/EmbeddedSpacePacket/include/space_packet.h"

int test_pus_frame_run_all(void);

static int test_pus_from_space_packet_tm(void) {
	uint8_t payload_data[] = {0xAA, 0xBB, 0xCC, 0xDD};
	pus_tm_sec_header_t tm_header = {
		.version = PUS_VERSION,
		.time_ref_status = 0,
		.serivce_type_id = 5,
		.subtype_id = 1,
		.msg_type_counter = 100,
		.destination_id = 1,
		.time = 12345,
		.spare = 0
	};

	sp_packet_t sp;
	sp_packet_init(&sp);
	sp_set_primary_header(&sp, 0, 0, 1, 100, SP_SEQ_FLAG_UNSEGMENTED, 0);
	sp_set_secondary_header(&sp, (uint8_t *)&tm_header, sizeof(tm_header));
	sp_set_payload(&sp, payload_data, sizeof(payload_data));

	pus_frame_t pus;
	int result = pus_from_space_packet(&sp, &pus);

	ASSERT_EQ_INT(1, result);
	ASSERT_EQ_INT(1, pus.is_tm);
	ASSERT_EQ_INT(sizeof(payload_data), pus.payload_len);
	ASSERT_EQ_MEM(payload_data, pus.payload, sizeof(payload_data));
	ASSERT_EQ_INT(tm_header.serivce_type_id, pus.sec_header.tm.serivce_type_id);

	return 0;
}

static int test_pus_from_space_packet_tc(void) {
	uint8_t payload_data[] = {0x11, 0x22, 0x33};
	pus_tc_sec_header_t tc_header = {
		.version = PUS_VERSION,
		.ack_flags = 0xF,
		.serivce_type_id = 17,
		.subtype_id = 1,
		.source_id = 5,
		.time = 54321,
		.spare = 0
	};

	sp_packet_t sp;
	sp_packet_init(&sp);
	sp_set_primary_header(&sp, 0, 1, 1, 200, SP_SEQ_FLAG_UNSEGMENTED, 0);
	sp_set_secondary_header(&sp, (uint8_t *)&tc_header, sizeof(tc_header));
	sp_set_payload(&sp, payload_data, sizeof(payload_data));

	pus_frame_t pus;
	int result = pus_from_space_packet(&sp, &pus);

	ASSERT_EQ_INT(1, result);
	ASSERT_EQ_INT(0, pus.is_tm);
	ASSERT_EQ_INT(sizeof(payload_data), pus.payload_len);
	ASSERT_EQ_MEM(payload_data, pus.payload, sizeof(payload_data));
	ASSERT_EQ_INT(tc_header.serivce_type_id, pus.sec_header.tc.serivce_type_id);

	return 0;
}

static int test_pus_from_space_packet_null_pointers(void) {
	sp_packet_t sp;
	pus_frame_t pus;

	int result1 = pus_from_space_packet(NULL, &pus);
	ASSERT_EQ_INT(0, result1);

	int result2 = pus_from_space_packet(&sp, NULL);
	ASSERT_EQ_INT(0, result2);

	return 0;
}

static int test_pus_from_space_packet_no_secondary_header(void) {
	uint8_t payload_data[] = {0x44, 0x55};

	sp_packet_t sp;
	sp_packet_init(&sp);
	sp_set_primary_header(&sp, 0, 0, 0, 50, SP_SEQ_FLAG_UNSEGMENTED, 0);
	sp_set_payload(&sp, payload_data, sizeof(payload_data));

	pus_frame_t pus;
	int result = pus_from_space_packet(&sp, &pus);

	ASSERT_EQ_INT(0, result);

	return 0;
}

static int test_pus_from_space_packet_invalid_version(void) {
	uint8_t header_data[] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t payload_data[] = {0xFF};

	sp_packet_t sp;
	sp_packet_init(&sp);
	sp_set_primary_header(&sp, 0, 0, 1, 75, SP_SEQ_FLAG_UNSEGMENTED, 0);
	sp_set_secondary_header(&sp, header_data, sizeof(header_data));
	sp_set_payload(&sp, payload_data, sizeof(payload_data));

	pus_frame_t pus;
	int result = pus_from_space_packet(&sp, &pus);

	ASSERT_EQ_INT(0, result);

	return 0;
}

static int test_pus_to_space_packet_tm(void) {
	uint8_t payload_data[] = {0x77, 0x88, 0x99};
	pus_tm_sec_header_t tm_header = {
		.version = PUS_VERSION,
		.time_ref_status = 1,
		.serivce_type_id = 3,
		.subtype_id = 25,
		.msg_type_counter = 500,
		.destination_id = 2,
		.time = 99999,
		.spare = 0
	};

	pus_frame_t pus = {
		.payload = payload_data,
		.payload_len = sizeof(payload_data),
		.is_tm = 1
	};
	memcpy(&pus.sec_header.tm, &tm_header, sizeof(tm_header));

	sp_packet_t sp;
	int result = pus_to_space_packet(&pus, &sp);

	ASSERT_EQ_INT(1, result);
	ASSERT_EQ_INT(0, sp.ph.type);
	ASSERT_EQ_INT(1, sp.ph.sec_hdr_flag);
	ASSERT_EQ_INT(sizeof(tm_header), sp.sec_hdr_len);
	ASSERT_EQ_INT(sizeof(payload_data), sp.payload_len);
	ASSERT_EQ_MEM(payload_data, sp.payload, sizeof(payload_data));

	return 0;
}

static int test_pus_to_space_packet_tc(void) {
	uint8_t payload_data[] = {0xDE, 0xAD, 0xBE, 0xEF};
	pus_tc_sec_header_t tc_header = {
		.version = PUS_VERSION,
		.ack_flags = 0xA,
		.serivce_type_id = 8,
		.subtype_id = 2,
		.source_id = 10,
		.time = 55555,
		.spare = 0
	};

	pus_frame_t pus = {
		.payload = payload_data,
		.payload_len = sizeof(payload_data),
		.is_tm = 0
	};
	memcpy(&pus.sec_header.tc, &tc_header, sizeof(tc_header));

	sp_packet_t sp;
	int result = pus_to_space_packet(&pus, &sp);

	ASSERT_EQ_INT(1, result);
	ASSERT_EQ_INT(1, sp.ph.type);
	ASSERT_EQ_INT(1, sp.ph.sec_hdr_flag);
	ASSERT_EQ_INT(sizeof(tc_header), sp.sec_hdr_len);
	ASSERT_EQ_INT(sizeof(payload_data), sp.payload_len);
	ASSERT_EQ_MEM(payload_data, sp.payload, sizeof(payload_data));

	return 0;
}

static int test_pus_to_space_packet_null_pointers(void) {
	pus_frame_t pus;
	sp_packet_t sp;

	int result1 = pus_to_space_packet(NULL, &sp);
	ASSERT_EQ_INT(0, result1);

	int result2 = pus_to_space_packet(&pus, NULL);
	ASSERT_EQ_INT(0, result2);

	return 0;
}

static int test_pus_roundtrip_conversion_tm(void) {
	uint8_t payload_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
	pus_tm_sec_header_t original_tm = {
		.version = PUS_VERSION,
		.time_ref_status = 2,
		.serivce_type_id = 1,
		.subtype_id = 3,
		.msg_type_counter = 42,
		.destination_id = 7,
		.time = 11111,
		.spare = 0
	};

	pus_frame_t original_pus = {
		.payload = payload_data,
		.payload_len = sizeof(payload_data),
		.is_tm = 1
	};
	memcpy(&original_pus.sec_header.tm, &original_tm, sizeof(original_tm));

	sp_packet_t sp;
	int result1 = pus_to_space_packet(&original_pus, &sp);
	ASSERT_EQ_INT(1, result1);

	pus_frame_t recovered_pus;
	int result2 = pus_from_space_packet(&sp, &recovered_pus);
	ASSERT_EQ_INT(1, result2);

	ASSERT_EQ_INT(original_pus.is_tm, recovered_pus.is_tm);
	ASSERT_EQ_INT(original_pus.payload_len, recovered_pus.payload_len);
	ASSERT_EQ_MEM(original_pus.payload, recovered_pus.payload, original_pus.payload_len);
	ASSERT_EQ_MEM(&original_pus.sec_header.tm, &recovered_pus.sec_header.tm,
	              sizeof(pus_tm_sec_header_t));

	return 0;
}

static int test_pus_roundtrip_conversion_tc(void) {
	uint8_t payload_data[] = {0xF0, 0xF1, 0xF2};
	pus_tc_sec_header_t original_tc = {
		.version = PUS_VERSION,
		.ack_flags = 0x5,
		.serivce_type_id = 6,
		.subtype_id = 8,
		.source_id = 15,
		.time = 22222,
		.spare = 0
	};

	pus_frame_t original_pus = {
		.payload = payload_data,
		.payload_len = sizeof(payload_data),
		.is_tm = 0
	};
	memcpy(&original_pus.sec_header.tc, &original_tc, sizeof(original_tc));

	sp_packet_t sp;
	int result1 = pus_to_space_packet(&original_pus, &sp);
	ASSERT_EQ_INT(1, result1);

	pus_frame_t recovered_pus;
	int result2 = pus_from_space_packet(&sp, &recovered_pus);
	ASSERT_EQ_INT(1, result2);

	ASSERT_EQ_INT(original_pus.is_tm, recovered_pus.is_tm);
	ASSERT_EQ_INT(original_pus.payload_len, recovered_pus.payload_len);
	ASSERT_EQ_MEM(original_pus.payload, recovered_pus.payload, original_pus.payload_len);
	ASSERT_EQ_MEM(&original_pus.sec_header.tc, &recovered_pus.sec_header.tc,
	              sizeof(pus_tc_sec_header_t));

	return 0;
}

int test_pus_frame_run_all(void) {
	RUN_TEST(test_pus_from_space_packet_tm);
	RUN_TEST(test_pus_from_space_packet_tc);
	RUN_TEST(test_pus_from_space_packet_null_pointers);
	RUN_TEST(test_pus_from_space_packet_no_secondary_header);
	RUN_TEST(test_pus_from_space_packet_invalid_version);
	RUN_TEST(test_pus_to_space_packet_tm);
	RUN_TEST(test_pus_to_space_packet_tc);
	RUN_TEST(test_pus_to_space_packet_null_pointers);
	RUN_TEST(test_pus_roundtrip_conversion_tm);
	RUN_TEST(test_pus_roundtrip_conversion_tc);

	return 0;
}
