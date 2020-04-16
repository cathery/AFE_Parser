#include "AFE.h"

static const char* GetAddressWithModBase(uint64_t module_base, uint64_t offset)
{
	static char string_buffer[51];
	int result;
	if (offset >= module_base && offset - module_base < 0x1'000'000)
		result = snprintf(string_buffer, sizeof(string_buffer), "0x%llx (MOD_BASE + 0x%llx)", offset, offset - module_base);
	else
		result = snprintf(string_buffer, sizeof(string_buffer), "0x%llx", offset);
	if (result <= 0)
	{
		return "OOPSIE WOOPSIE";
	}
	return string_buffer;
}

static void PrintReportDesc(uint32_t magic, uint32_t error_desc, uint64_t titleid)
{
	printf("Magic: %.4s (0x%lX)\n", (char*)&magic, magic);
	printf("Error description: 0x%lX\n", error_desc);
	printf("Title ID: %llX\n", titleid);
}

static void PrintRegisters(uint64_t *gprs, uint64_t pc, uint64_t module_base = -1)
{
	printf("Registers:\n");
	for (int i = 0; i != 29; ++i)
	{
		printf("	X[%02i]: %s\n", i, GetAddressWithModBase(module_base, gprs[i]));
	}
	printf("	FP: %s\n", GetAddressWithModBase(module_base, gprs[29]));
	printf("	LR: %s\n", GetAddressWithModBase(module_base, gprs[30]));
	printf("	SP: %s\n", GetAddressWithModBase(module_base, gprs[31]));
	printf("	PC: %s\n", GetAddressWithModBase(module_base, pc));

}

static void PrintMisc(uint32_t pstate, uint32_t afsr0, uint32_t afsr1, uint32_t esr, uint64_t far, uint64_t report_identifier)
{
	printf("pstate: 0x%lx\n", pstate);
	printf("afsr0: 0x%lx\n", afsr0);
	printf("afsr1: 0x%lx\n", afsr1);
	printf("esr: 0x%lx\n", esr);
	printf("far: 0x%llx\n", far);
	printf("Report identifier: 0x%llx\n", report_identifier);
}

static void PrintStackTrace(uint64_t* stack_trace, uint64_t stack_trace_size, uint64_t module_base = -1)
{
	printf("Stack trace:\n");
	for (uint64_t i = 0; i != stack_trace_size; ++i)
	{
		//For whatever reason doing this in a single line doesn't work properly
		printf("	ReturnAddress[%02llu]: ", i);
		printf("%s\n", GetAddressWithModBase(module_base, stack_trace[i]));
	}
}

static void PrintStackDump(uint8_t* stack_dump, uint64_t stack_dump_size, uint64_t stack_dump_base = 0)
{
	printf("Stack Dump:	00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n"
		"---------------------------------------------------------------\n");
	for (uint64_t i = 0; i < stack_dump_size; i += 0x10)
	{
		printf("%06llx		%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i + stack_dump_base,
			stack_dump[i + 0], stack_dump[i + 1], stack_dump[i + 2], stack_dump[i + 3],
			stack_dump[i + 4], stack_dump[i + 5], stack_dump[i + 6], stack_dump[i + 7],
			stack_dump[i + 8], stack_dump[i + 9], stack_dump[i + 10], stack_dump[i + 11],
			stack_dump[i + 12], stack_dump[i + 13], stack_dump[i + 14], stack_dump[i + 15]);
	}
}

static void PrintTlsDump(uint8_t* tls_dump, uint64_t tls_dump_size = AMS_FATAL_ERROR_TLS_SIZE, uint64_t tls_dump_base = 0)
{
	printf("TLS Dump:	00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n"
		"---------------------------------------------------------------\n");
	for (uint64_t i = 0; i < tls_dump_size; i += 0x10)
	{
		printf("%06llx		%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i + tls_dump_base,
			tls_dump[i + 0], tls_dump[i + 1], tls_dump[i + 2], tls_dump[i + 3],
			tls_dump[i + 4], tls_dump[i + 5], tls_dump[i + 6], tls_dump[i + 7],
			tls_dump[i + 8], tls_dump[i + 9], tls_dump[i + 10], tls_dump[i + 11],
			tls_dump[i + 12], tls_dump[i + 13], tls_dump[i + 14], tls_dump[i + 15]);
	}
}

void PrintAFE0Report(atmosphere_fatal_error_ctx_0* fatal_report)
{
	printf("Fatal report (AFE0):\n");
	PrintReportDesc(fatal_report->magic, fatal_report->error_desc, fatal_report->title_id);
	printf("\n");

	PrintRegisters(fatal_report->gprs, fatal_report->pc);
	printf("\n");

	PrintMisc(fatal_report->pstate, fatal_report->afsr0, fatal_report->afsr1, fatal_report->esr, fatal_report->far, fatal_report->report_identifier);
	printf("\n");
}

void PrintAFE1Report(atmosphere_fatal_error_ctx_1* fatal_report)
{
	printf("Fatal report (AFE1):\n");
	PrintReportDesc(fatal_report->magic, fatal_report->error_desc, fatal_report->title_id);
	printf("\n");

	PrintRegisters(fatal_report->gprs, fatal_report->pc, fatal_report->module_base);
	printf("\n");

	PrintMisc(fatal_report->pstate, fatal_report->afsr0, fatal_report->afsr1, fatal_report->esr, fatal_report->far, fatal_report->report_identifier);
	printf("\n");

	PrintStackTrace(fatal_report->stack_trace, fatal_report->stack_trace_size, fatal_report->module_base);
	printf("\n");

	PrintStackDump(fatal_report->stack_dump, fatal_report->stack_dump_size);
	printf("\n");
}

void PrintAFE2Report(atmosphere_fatal_error_ctx* fatal_report)
{
	printf("Fatal report (AFE2):\n");
	PrintReportDesc(fatal_report->magic, fatal_report->error_desc, fatal_report->title_id);
	printf("\n");

	PrintRegisters(fatal_report->gprs, fatal_report->pc, fatal_report->module_base);
	printf("\n");

	PrintMisc(fatal_report->pstate, fatal_report->afsr0, fatal_report->afsr1, fatal_report->esr, fatal_report->far, fatal_report->report_identifier);
	printf("\n");

	PrintStackTrace(fatal_report->stack_trace, fatal_report->stack_trace_size, fatal_report->module_base);
	printf("\n");

	PrintStackDump(fatal_report->stack_dump, fatal_report->stack_dump_size);
	printf("\n");

	PrintTlsDump(fatal_report->tls);
	printf("\n");
}