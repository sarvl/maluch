#include "Vio_bridge.h"
#include "verilated.h"
#include <cstdint>
#include <cstdio>

int failed_tests = 0;

#define CHECK(cond, msg)                                   \
	do {                                                   \
		if (!(cond)) {                                     \
			printf("====[FAIL]==== %s <--------\n", msg); \
			failed_tests++;                                \
		}                                                  \
	} while (0)


void test_flags_mapping(Vio_bridge* top)
{
	printf("Starting FLAGS mapping test\n");

	top->tim2io_int_f = 1;
	top->kb2io_int_f = 0;
	top->gpu2io_int_f = 1;

	top->tim2io_busy_f = 0;
	top->kb2io_busy_f = 1;
	top->gpu2io_busy_f = 1;
	top->eval();

	CHECK(top->io2core_int_f == 0b10100000, "FLAGS: io2core_int_f mapping mismatch");
	CHECK(top->io2core_busy_f == 0b01100000, "FLAGS: io2core_busy_f mapping mismatch");

	// Lower five bits are reserved and should stay zero.
	CHECK((top->io2core_int_f & 0x1F) == 0, "FLAGS: io2core_int_f[4:0] should be 0");
	CHECK((top->io2core_busy_f & 0x1F) == 0, "FLAGS: io2core_busy_f[4:0] should be 0");

	printf("[FINISHED] FLAGS mapping\n\n");
}

void test_read_routing(Vio_bridge* top)
{
	printf("Starting READ routing test\n");

	top->core2io_r_en = 1;
	top->core2io_w_en = 0;

	top->core2io_addr = 0b000;
	top->eval();
	CHECK(top->io2tim_r_en == 1, "READ: timer read enable should be asserted");
	CHECK(top->io2kb_r_en == 0, "READ: keyboard read enable should be deasserted");
	CHECK(top->io2gpu_w_en == 0, "READ: gpu write enable should be deasserted");

	top->core2io_addr = 0b001;
	top->eval();
	CHECK(top->io2tim_r_en == 0, "READ: timer read enable should be deasserted");
	CHECK(top->io2kb_r_en == 1, "READ: keyboard read enable should be asserted");
	CHECK(top->io2gpu_w_en == 0, "READ: gpu write enable should be deasserted");

	top->core2io_addr = 0b010;
	top->eval();
	CHECK(top->io2tim_r_en == 0, "READ: timer read enable should be deasserted for gpu addr");
	CHECK(top->io2kb_r_en == 0, "READ: keyboard read enable should be deasserted for gpu addr");

	top->core2io_addr = 0b111;
	top->eval();
	CHECK(top->io2tim_r_en == 0, "READ: timer read enable should be 0 for default addr");
	CHECK(top->io2kb_r_en == 0, "READ: keyboard read enable should be 0 for default addr");

	printf("[FINISHED] READ routing\n\n");
}

void test_write_routing(Vio_bridge* top)
{
	printf("Starting WRITE routing test\n");

	top->core2io_r_en = 0;
	top->core2io_w_en = 1;
	top->core2io_data_w = 0xBEEF;

	top->core2io_addr = 0b010;
	top->eval();
	CHECK(top->io2gpu_w_en == 1, "WRITE: gpu write enable should be asserted");
	CHECK(top->io2gpu_data_w == 0xBEEF, "WRITE: gpu data mismatch");
	CHECK(top->io2tim_r_en == 0, "WRITE: timer read enable should be deasserted");
	CHECK(top->io2kb_r_en == 0, "WRITE: keyboard read enable should be deasserted");

	top->core2io_addr = 0b001;
	top->eval();
	CHECK(top->io2gpu_w_en == 0, "WRITE: gpu write enable should be 0 for non-gpu addr");
	CHECK(top->io2gpu_data_w == 0, "WRITE: gpu data should be 0 for non-gpu addr");

	top->core2io_addr = 0b010;
	top->core2io_w_en = 0;
	top->eval();
	CHECK(top->io2gpu_w_en == 0, "WRITE: gpu write enable should follow core2io_w_en");

	printf("[FINISHED] WRITE routing\n\n");
}

void test_data_mux_to_core(Vio_bridge* top)
{
	printf("Starting DATA mux-to-core test\n");

	top->tim2io_data_r = 0x1234;
	top->kb2io_data_r = 0xABCD;

	top->core2io_addr = 0b000;
	top->eval();
	CHECK(top->io2core_data_r == 0x1234, "MUX: io2core_data_r should come from timer");

	top->core2io_addr = 0b001;
	top->eval();
	CHECK(top->io2core_data_r == 0xABCD, "MUX: io2core_data_r should come from keyboard");

	top->core2io_addr = 0b010;
	top->eval();
	CHECK(top->io2core_data_r == 0x0000, "MUX: io2core_data_r should be 0 for gpu addr");

	top->core2io_addr = 0b111;
	top->eval();
	CHECK(top->io2core_data_r == 0x0000, "MUX: io2core_data_r should be 0 for default addr");

	printf("[FINISHED] DATA mux-to-core\n\n");
}

int main(int argc, char** argv)
{
	Verilated::commandArgs(argc, argv);
	Vio_bridge* top = new Vio_bridge;

	test_flags_mapping(top);
	test_read_routing(top);
	test_write_routing(top);
	test_data_mux_to_core(top);

	printf("=====================================\n");
	printf("Simulation completed\n");
	printf("Failed tests: %d\n", failed_tests);
	printf("=====================================\n");

	delete top;
	return failed_tests == 0 ? 0 : 1;
}
