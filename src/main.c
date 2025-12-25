// src/main.c
/**
 * bit(n) Emulator - Main Entry Point
 * Supports RP2040 and RP2350 targets
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef RP2040_TARGET
    #include "rp2040/rp2040.h"
    typedef rp2040_system_t system_t;
#elif defined(RP2350_ARM_MODE)
    #include "rp2350/rp2350.h"
    typedef rp2350_system_t system_t;
#elif defined(RP2350_RISCV_MODE)
    #include "rp2350/rp2350.h"
    typedef rp2350_system_t system_t;
#else
    #error "No target defined! Use -DRP2040_TARGET or -DRP2350_ARM_MODE or -DRP2350_RISCV_MODE"
#endif

typedef struct {
    char *elf_file;
    uint64_t max_cycles;
    bool verbose;
    int gdb_port;
} cli_args_t;

void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -f FILE    Load ELF file\n");
    printf("  -c CYCLES  Run for N cycles\n");
    printf("  -v         Verbose output\n");
    printf("  -g PORT    Enable GDB remote debugging on PORT\n");
    printf("  -h         Show this help\n");
}

cli_args_t parse_args(int argc, char *argv[])
{
    cli_args_t args = {
        .elf_file = NULL,
        .max_cycles = 1000000,
        .verbose = false,
        .gdb_port = -1,
    };
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            args.elf_file = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            args.max_cycles = strtoll(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "-v") == 0) {
            args.verbose = true;
        } else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
            args.gdb_port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            exit(0);
        }
    }
    
    return args;
}

void print_banner(void)
{
    printf("╔═══════════════════════════════════════╗\n");
    printf("║  bit(n) - Microcontroller Emulator   ║\n");
    printf("║  ARM Cortex-M & RISC-V Support       ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    printf("\n");
    
#ifdef RP2040_TARGET
    printf("Target: RP2040 (dual ARM Cortex-M0+ @ 133MHz)\n");
    printf("SRAM: 264KB | GPIO: 30 pins | PIO: 2 blocks\n");
#elif defined(RP2350_ARM_MODE)
    printf("Target: RP2350 ARM (dual ARM Cortex-M33 @ 150MHz)\n");
    printf("SRAM: 520KB | GPIO: 48 pins | PIO: 3 blocks\n");
#elif defined(RP2350_RISCV_MODE)
    printf("Target: RP2350 RISC-V (dual Hazard3 @ 150MHz)\n");
    printf("SRAM: 520KB | GPIO: 48 pins | PIO: 3 blocks\n");
#endif
    printf("\n");
}

int main(int argc, char *argv[])
{
    print_banner();
    
    cli_args_t args = parse_args(argc, argv);
    
    if (!args.elf_file) {
        fprintf(stderr, "Error: No ELF file specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    if (args.verbose) {
        printf("Loading ELF: %s\n", args.elf_file);
        printf("Max cycles: %llu\n", (unsigned long long)args.max_cycles);
        printf("\n");
    }
    
    /* Create system */
#ifdef RP2040_TARGET
    system_t *sys = rp2040_create();
#else
    system_t *sys = rp2350_create(NULL);  // TODO: add config struct
#endif
    
    if (!sys) {
        fprintf(stderr, "Failed to create emulator system\n");
        return 1;
    }
    
    /* Load ELF file */
#ifdef RP2040_TARGET
    if (rp2040_load_elf(sys, args.elf_file) < 0) {
        fprintf(stderr, "Failed to load ELF file\n");
        rp2040_destroy(sys);
        return 1;
    }
#else
    if (rp2350_load_elf(sys, args.elf_file) < 0) {
        fprintf(stderr, "Failed to load ELF file\n");
        rp2350_destroy(sys);
        return 1;
    }
#endif
    
    if (args.verbose) {
        printf("✓ ELF file loaded successfully\n");
        printf("✓ Starting emulation...\n\n");
    }
    
    /* Run emulation */
#ifdef RP2040_TARGET
    int result = rp2040_run_cycles(sys, args.max_cycles);
#else
    int result = rp2350_run_cycles(sys, args.max_cycles);
#endif
    
    if (result < 0) {
        fprintf(stderr, "Emulation error\n");
        goto cleanup;
    }
    
    if (args.verbose) {
        printf("\n✓ Emulation completed\n");
#ifdef RP2040_TARGET
        printf("Cycles executed: %llu\n", (unsigned long long)sys->cycle_count);
        printf("Core 0 PC: 0x%08x\n", sys->cores[0]->pc);
        printf("Core 0 SP: 0x%08x\n", sys->cores[0]->sp);
#endif
    }
    
cleanup:
#ifdef RP2040_TARGET
    rp2040_destroy(sys);
#else
    rp2350_destroy(sys);
#endif
    
    return result < 0 ? 1 : 0;
}
