#include "cunit.h"
#include "../include/pus.h"

int test_pus_codec_run_all(void);

static int test_tc_sec_header_decode_reads_wire_fields(void)
{
	const uint8_t data[PUS_TC_SEC_HEADER_LEN] = {
		0x1fu, 0x11u, 0x01u, 0x12u, 0x34u,
		0x01u, 0x02u, 0x03u, 0x04u, 0x00u};
	pus_tc_sec_header_t header = {0};

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tc_sec_header_decode(
									 data, sizeof(data), &header));
	ASSERT_EQ_INT(1, header.version);
	ASSERT_EQ_INT(0x0f, header.ack_flags);
	ASSERT_EQ_INT(17, header.service_type_id);
	ASSERT_EQ_INT(1, header.subtype_id);
	ASSERT_EQ_INT(0x1234, header.source_id);
	ASSERT_EQ_INT(0x01020304, header.time);
	ASSERT_EQ_INT(0, header.spare);

	return 0;
}

static int test_tm_sec_header_encode_writes_wire_fields(void)
{
	const uint8_t expected[PUS_TM_SEC_HEADER_LEN] = {
		0x12u, 0x05u, 0x03u, 0x12u, 0x34u, 0xabu,
		0xcdu, 0x01u, 0x02u, 0x03u, 0x04u, 0x00u};
	uint8_t out[PUS_TM_SEC_HEADER_LEN] = {0};
	uint16_t out_len = 0u;
	pus_tm_sec_header_t header = {0};

	header.version = 1u;
	header.time_ref_status = 2u;
	header.service_type_id = PUS_SERVICE_EVENT_REPORTING;
	header.subtype_id = PUS_SUBTYPE_EVENT_MEDIUM_SEVERITY;
	header.msg_type_counter = 0x1234u;
	header.destination_id = 0xabcdu;
	header.time = 0x01020304u;
	header.spare = 0u;

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tm_sec_header_encode(
									 &header, out, sizeof(out), &out_len));
	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN, out_len);
	ASSERT_EQ_MEM(expected, out, sizeof(expected));

	return 0;
}

static int test_tc_sec_header_encode_round_trips_through_decode(void)
{
	uint8_t out[PUS_TC_SEC_HEADER_LEN] = {0};
	uint16_t out_len = 0u;
	pus_tc_sec_header_t encoded = {0};
	pus_tc_sec_header_t decoded = {0};

	encoded.version = 1u;
	encoded.ack_flags = 0x05u;
	encoded.service_type_id = PUS_SERVICE_TEST;
	encoded.subtype_id = PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION;
	encoded.source_id = 0x4567u;
	encoded.time = 0x09abcdefu;
	encoded.spare = 0xaau;

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tc_sec_header_encode(
									 &encoded, out, sizeof(out), &out_len));
	ASSERT_EQ_INT(PUS_TC_SEC_HEADER_LEN, out_len);
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tc_sec_header_decode(
									 out, out_len, &decoded));
	ASSERT_EQ_INT(encoded.version, decoded.version);
	ASSERT_EQ_INT(encoded.ack_flags, decoded.ack_flags);
	ASSERT_EQ_INT(encoded.service_type_id, decoded.service_type_id);
	ASSERT_EQ_INT(encoded.subtype_id, decoded.subtype_id);
	ASSERT_EQ_INT(encoded.source_id, decoded.source_id);
	ASSERT_EQ_INT(encoded.time, decoded.time);
	ASSERT_EQ_INT(encoded.spare, decoded.spare);

	return 0;
}

static int test_tm_sec_header_decode_round_trips_through_encode(void)
{
	uint8_t out[PUS_TM_SEC_HEADER_LEN] = {0};
	uint16_t out_len = 0u;
	pus_tm_sec_header_t encoded = {0};
	pus_tm_sec_header_t decoded = {0};

	encoded.version = 1u;
	encoded.time_ref_status = 0x03u;
	encoded.service_type_id = PUS_SERVICE_HOUSEKEEPING;
	encoded.subtype_id = PUS_SUBTYPE_HOUSEKEEPING_PARAMETER_REPORT;
	encoded.msg_type_counter = 0x1001u;
	encoded.destination_id = 0x2222u;
	encoded.time = 0x3456789au;
	encoded.spare = 0x55u;

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tm_sec_header_encode(
									 &encoded, out, sizeof(out), &out_len));
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tm_sec_header_decode(
									 out, out_len, &decoded));
	ASSERT_EQ_INT(encoded.version, decoded.version);
	ASSERT_EQ_INT(encoded.time_ref_status, decoded.time_ref_status);
	ASSERT_EQ_INT(encoded.service_type_id, decoded.service_type_id);
	ASSERT_EQ_INT(encoded.subtype_id, decoded.subtype_id);
	ASSERT_EQ_INT(encoded.msg_type_counter, decoded.msg_type_counter);
	ASSERT_EQ_INT(encoded.destination_id, decoded.destination_id);
	ASSERT_EQ_INT(encoded.time, decoded.time);
	ASSERT_EQ_INT(encoded.spare, decoded.spare);

	return 0;
}

static int test_sec_header_codec_rejects_bad_inputs(void)
{
	uint8_t tc_data[PUS_TC_SEC_HEADER_LEN] = {0};
	uint8_t tm_data[PUS_TM_SEC_HEADER_LEN] = {0};
	uint16_t out_len = 0u;
	pus_tc_sec_header_t tc_header = {0};
	pus_tm_sec_header_t tm_header = {0};

	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_sec_header_decode(
									   NULL, sizeof(tc_data), &tc_header));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tm_sec_header_encode(
									   &tm_header, NULL, sizeof(tm_data), &out_len));
	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH, pus_tc_sec_header_decode(
											 tc_data, PUS_TC_SEC_HEADER_LEN - 1u, &tc_header));
	ASSERT_EQ_INT(PUS_STATUS_BUFFER_TOO_SMALL, pus_tm_sec_header_encode(
												   &tm_header, tm_data, PUS_TM_SEC_HEADER_LEN - 1u, &out_len));

	return 0;
}

int test_pus_codec_run_all(void)
{
	RUN_TEST(test_tc_sec_header_decode_reads_wire_fields);
	RUN_TEST(test_tm_sec_header_encode_writes_wire_fields);
	RUN_TEST(test_tc_sec_header_encode_round_trips_through_decode);
	RUN_TEST(test_tm_sec_header_decode_round_trips_through_encode);
	RUN_TEST(test_sec_header_codec_rejects_bad_inputs);

	return 0;
}
