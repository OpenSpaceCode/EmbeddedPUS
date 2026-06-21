#include "cunit.h"
#include "test_runners.h"
#include "pus_codec.h"
#include <string.h>

/* ---- TC secondary header ---- */

static int test_tc_encode(void)
{
	pus_tc_sec_header_t hdr = {0};
	uint8_t  out[PUS_TC_SEC_HEADER_LEN];
	uint16_t len = 0;

	hdr.version         = 1u;
	hdr.ack_flags       = 0x09u; /* acceptance + completion */
	hdr.service_type_id = 17u;
	hdr.subtype_id      = 1u;
	hdr.source_id       = 0x0042u;
	hdr.time            = 0x00001234u;
	hdr.spare           = 0u;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_tc_sec_header_encode(&hdr, out, sizeof(out), &len));
	ASSERT_EQ_INT(PUS_TC_SEC_HEADER_LEN, len);

	ASSERT_EQ_INT(0x19, out[0]);  /* (1<<4)|0x09 */
	ASSERT_EQ_INT(17,   out[1]);
	ASSERT_EQ_INT(1,    out[2]);
	ASSERT_EQ_INT(0x00, out[3]);
	ASSERT_EQ_INT(0x42, out[4]);
	ASSERT_EQ_INT(0x00, out[5]);
	ASSERT_EQ_INT(0x00, out[6]);
	ASSERT_EQ_INT(0x12, out[7]);
	ASSERT_EQ_INT(0x34, out[8]);
	ASSERT_EQ_INT(0x00, out[9]);

	return 0;
}

static int test_tc_decode(void)
{
	const uint8_t raw[PUS_TC_SEC_HEADER_LEN] =
		{ 0x19, 17, 1, 0x00, 0x42, 0x00, 0x00, 0x12, 0x34, 0x00 };
	pus_tc_sec_header_t hdr;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_tc_sec_header_decode(raw, sizeof(raw), &hdr));

	ASSERT_EQ_INT(1u,     hdr.version);
	ASSERT_EQ_INT(0x09u,  hdr.ack_flags);
	ASSERT_EQ_INT(17u,    hdr.service_type_id);
	ASSERT_EQ_INT(1u,     hdr.subtype_id);
	ASSERT_EQ_INT(0x0042, hdr.source_id);
	ASSERT_EQ_INT(0x1234, hdr.time);

	return 0;
}

static int test_tc_roundtrip(void)
{
	pus_tc_sec_header_t orig = { 1u, 0x0Fu, 3u, 25u, 0xABCDu, 0xDEADBEEFu, 0u };
	pus_tc_sec_header_t decoded = {0};
	uint8_t  buf[PUS_TC_SEC_HEADER_LEN];
	uint16_t len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_tc_sec_header_encode(&orig, buf, sizeof(buf), &len));
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_tc_sec_header_decode(buf, len, &decoded));

	ASSERT_EQ_INT(orig.version,         decoded.version);
	ASSERT_EQ_INT(orig.ack_flags,       decoded.ack_flags);
	ASSERT_EQ_INT(orig.service_type_id, decoded.service_type_id);
	ASSERT_EQ_INT(orig.subtype_id,      decoded.subtype_id);
	ASSERT_EQ_INT(orig.source_id,       decoded.source_id);
	ASSERT_EQ_INT((int)orig.time,       (int)decoded.time);

	return 0;
}

/* ---- TM secondary header ---- */

static int test_tm_encode(void)
{
	pus_tm_sec_header_t hdr = {0};
	uint8_t  out[PUS_TM_SEC_HEADER_LEN];
	uint16_t len = 0;

	hdr.version          = 1u;
	hdr.time_ref_status  = 0u;
	hdr.service_type_id  = 5u;
	hdr.subtype_id       = 1u;
	hdr.msg_type_counter = 0x0007u;
	hdr.destination_id   = 0x0100u;
	hdr.time             = 0x00001000u;
	hdr.spare            = 0u;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_tm_sec_header_encode(&hdr, out, sizeof(out), &len));
	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN, len);

	ASSERT_EQ_INT(0x10, out[0]);  /* (1<<4)|0 */
	ASSERT_EQ_INT(5,    out[1]);
	ASSERT_EQ_INT(1,    out[2]);
	ASSERT_EQ_INT(0x00, out[3]);
	ASSERT_EQ_INT(0x07, out[4]);
	ASSERT_EQ_INT(0x01, out[5]);
	ASSERT_EQ_INT(0x00, out[6]);
	ASSERT_EQ_INT(0x00, out[7]);
	ASSERT_EQ_INT(0x00, out[8]);
	ASSERT_EQ_INT(0x10, out[9]);
	ASSERT_EQ_INT(0x00, out[10]);
	ASSERT_EQ_INT(0x00, out[11]);

	return 0;
}

static int test_tm_decode(void)
{
	const uint8_t raw[PUS_TM_SEC_HEADER_LEN] =
		{ 0x10, 5, 1, 0x00, 0x07, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00 };
	pus_tm_sec_header_t hdr;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_tm_sec_header_decode(raw, sizeof(raw), &hdr));

	ASSERT_EQ_INT(1u,     hdr.version);
	ASSERT_EQ_INT(0u,     hdr.time_ref_status);
	ASSERT_EQ_INT(5u,     hdr.service_type_id);
	ASSERT_EQ_INT(1u,     hdr.subtype_id);
	ASSERT_EQ_INT(0x0007, hdr.msg_type_counter);
	ASSERT_EQ_INT(0x0100, hdr.destination_id);
	ASSERT_EQ_INT(0x1000, (int)hdr.time);

	return 0;
}

static int test_null_returns_null(void)
{
	uint8_t  buf[32];
	uint16_t len;
	pus_tc_sec_header_t tc_hdr = {0};
	pus_tm_sec_header_t tm_hdr = {0};

	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_sec_header_decode(NULL, 10, &tc_hdr));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_sec_header_decode(buf,  10, NULL));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_sec_header_encode(NULL, buf, 32, &len));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_sec_header_encode(&tc_hdr, NULL, 32, &len));

	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tm_sec_header_decode(NULL, 12, &tm_hdr));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tm_sec_header_encode(NULL, buf, 32, &len));

	return 0;
}

static int test_buffer_too_small(void)
{
	pus_tc_sec_header_t tc_hdr = {0};
	pus_tm_sec_header_t tm_hdr = {0};
	uint8_t  small[4] = {0};
	uint16_t len;

	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH,
		pus_tc_sec_header_decode(small, sizeof(small), &tc_hdr));
	ASSERT_EQ_INT(PUS_STATUS_BUFFER_TOO_SMALL,
		pus_tc_sec_header_encode(&tc_hdr, small, sizeof(small), &len));

	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH,
		pus_tm_sec_header_decode(small, sizeof(small), &tm_hdr));
	ASSERT_EQ_INT(PUS_STATUS_BUFFER_TOO_SMALL,
		pus_tm_sec_header_encode(&tm_hdr, small, sizeof(small), &len));

	return 0;
}

pus_test_result_t test_pus_codec_run_all(void)
{
	RUN_TEST(test_tc_encode);
	RUN_TEST(test_tc_decode);
	RUN_TEST(test_tc_roundtrip);
	RUN_TEST(test_tm_encode);
	RUN_TEST(test_tm_decode);
	RUN_TEST(test_null_returns_null);
	RUN_TEST(test_buffer_too_small);
	return (pus_test_result_t){ cunit_total_tests - cunit_overall_failures, cunit_total_tests };
}
