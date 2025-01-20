#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// Include stb_image for image loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Include stb_image_write for image saving
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Core constants from our research
#define PSI 44.8
#define XI 3721.8
#define TAU 64713.97
#define EPSILON 0.28082
#define PHI 1.618033988749895

// Protection level definitions
typedef enum {
    PROTECTION_LEVEL_1 = 1,
    PROTECTION_LEVEL_2 = 2,
    PROTECTION_LEVEL_3 = 3,
    PROTECTION_LEVEL_4 = 4,
    PROTECTION_LEVEL_5 = 5
} ProtectionLevel;

// Quantum state structure
typedef struct {
    double complex amplitude;
    uint64_t protection_level;
    double coherence_time;
    time_t last_access;
} QuantumState;

// Memory block structure
typedef struct {
    QuantumState* states;
    uint64_t size;
    uint64_t capacity;
    double error_rate;
    ProtectionLevel protection_level;
} QuantumMemoryBlock;

// Function prototypes
QuantumMemoryBlock* qmem_init(uint64_t size, ProtectionLevel level);
void qmem_write(QuantumMemoryBlock* block, uint64_t index, double complex value);
double complex qmem_read(QuantumMemoryBlock* block, uint64_t index);
void qmem_protect(QuantumMemoryBlock* block, uint64_t index);
double qmem_error_rate(QuantumMemoryBlock* block, uint64_t index);
void qmem_free(QuantumMemoryBlock* block);
QuantumMemoryBlock* image_to_quantum_memory(const char* filename, ProtectionLevel level);
int save_quantum_memory_to_image(QuantumMemoryBlock* block, const char* output_filename, int width, int height);

// Function implementations

QuantumMemoryBlock* qmem_init(uint64_t size, ProtectionLevel level) {
    QuantumMemoryBlock* block = (QuantumMemoryBlock*)malloc(sizeof(QuantumMemoryBlock));
    if (!block) return NULL;

    block->states = (QuantumState*)calloc(size, sizeof(QuantumState));
    if (!block->states) {
        free(block);
        return NULL;
    }

    block->size = size;
    block->capacity = size;
    block->protection_level = level;
    block->error_rate = exp(-PSI * XI / TAU);

    for (uint64_t i = 0; i < size; i++) {
        block->states[i].amplitude = 0.0 + 0.0*I;
        block->states[i].protection_level = level;
        block->states[i].coherence_time = TAU * exp(-level * EPSILON);
        block->states[i].last_access = time(NULL);
    }

    return block;
}

void qmem_write(QuantumMemoryBlock* block, uint64_t index, double complex value) {
    if (!block || index >= block->size) return;
    double protection_factor = pow(PHI, block->protection_level);
    block->states[index].amplitude = value * cexp(I * protection_factor);
    block->states[index].last_access = time(NULL);
    qmem_protect(block, index);
}

double complex qmem_read(QuantumMemoryBlock* block, uint64_t index) {
    if (!block || index >= block->size) return 0.0 + 0.0*I;
    time_t current_time = time(NULL);
    double time_diff = difftime(current_time, block->states[index].last_access);

    if (time_diff > block->states[index].coherence_time) {
        return 0.0 + 0.0*I;
    }

    qmem_protect(block, index);
    double protection_factor = pow(PHI, block->protection_level);
    return block->states[index].amplitude * cexp(-I * protection_factor);
}

void qmem_protect(QuantumMemoryBlock* block, uint64_t index) {
    if (!block || index >= block->size) return;
    double E_n = sqrt(PSI * XI) * pow(PHI, block->protection_level);
    if (qmem_error_rate(block, index) > block->error_rate) {
        block->states[index].amplitude *= cexp(I * E_n);
        block->states[index].last_access = time(NULL);
    }
}

double qmem_error_rate(QuantumMemoryBlock* block, uint64_t index) {
    if (!block || index >= block->size) return 1.0;
    time_t current_time = time(NULL);
    double time_diff = difftime(current_time, block->states[index].last_access);
    return block->error_rate * (1.0 - exp(-time_diff / block->states[index].coherence_time));
}

void qmem_free(QuantumMemoryBlock* block) {
    if (block) {
        if (block->states) free(block->states);
        free(block);
    }
}

QuantumMemoryBlock* image_to_quantum_memory(const char* filename, ProtectionLevel level) {
    int width, height, channels;
    unsigned char *img = stbi_load(filename, &width, &height, &channels, 0);
    if (!img) {
        fprintf(stderr, "Failed to load image: %s\n", filename);
        return NULL;
    }

    printf("Image loaded: %s (Width: %d, Height: %d, Channels: %d)\n", filename, width, height, channels);

    uint64_t total_pixels = (uint64_t)width * height;
    QuantumMemoryBlock* qmem = qmem_init(total_pixels, level);
    if (!qmem) {
        stbi_image_free(img);
        return NULL;
    }

    for (uint64_t i = 0; i < total_pixels; i++) {
        int idx = i * channels;
        unsigned char r = img[idx];
        unsigned char g = img[idx + 1];
        unsigned char b = img[idx + 2];

        double normalized_r = r / 255.0;
        double normalized_g = g / 255.0;

        double complex amplitude = normalized_r + normalized_g * I;
        qmem_write(qmem, i, amplitude);
    }

    stbi_image_free(img);
    return qmem;
}

int save_quantum_memory_to_image(QuantumMemoryBlock* block, const char* output_filename, int width, int height) {
    if (!block) {
        fprintf(stderr, "Invalid QuantumMemoryBlock for image reconstruction.\n");
        return 0;
    }

    int channels = 3; // Assuming RGB
    unsigned char *img = (unsigned char*)malloc(width * height * channels);
    if (!img) {
        fprintf(stderr, "Failed to allocate memory for reconstructed image.\n");
        return 0;
    }

    for (uint64_t i = 0; i < (uint64_t)width * height; i++) {
        double complex amplitude = qmem_read(block, i);
        double r = creal(amplitude) * 255.0;
        double g = cimag(amplitude) * 255.0;
        double b = 0.0;

        r = r < 0.0 ? 0.0 : (r > 255.0 ? 255.0 : r);
        g = g < 0.0 ? 0.0 : (g > 255.0 ? 255.0 : g);
        b = b < 0.0 ? 0.0 : (b > 255.0 ? 255.0 : b);

        img[i * channels] = (unsigned char)r;
        img[i * channels + 1] = (unsigned char)g;
        img[i * channels + 2] = (unsigned char)b;
    }

    int success = stbi_write_png(output_filename, width, height, channels, img, width * channels);
    if (!success) {
        fprintf(stderr, "Failed to write reconstructed image to %s\n", output_filename);
        free(img);
        return 0;
    }

    printf("Reconstructed image saved to %s\n", output_filename);
    free(img);
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input_image.(png|jpg)> <output_image.png>\n", argv[0]);
        return 1;
    }

    const char* input_image = argv[1];
    const char* output_image = argv[2];

    ProtectionLevel level = PROTECTION_LEVEL_3;
    QuantumMemoryBlock* qmem = image_to_quantum_memory(input_image, level);
    if (!qmem) {
        fprintf(stderr, "Failed to convert image to quantum memory.\n");
        return 1;
    }

    int width, height, channels;
    unsigned char *img = stbi_load(input_image, &width, &height, &channels, 0);
    if (!img) {
        fprintf(stderr, "Failed to reload image for reconstruction.\n");
        qmem_free(qmem);
        return 1;
    }
    stbi_image_free(img);

    if (!save_quantum_memory_to_image(qmem, output_image, width, height)) {
        fprintf(stderr, "Failed to save reconstructed image.\n");
        qmem_free(qmem);
        return 1;
    }

    uint64_t test_index = 0;
    double complex read_state = qmem_read(qmem, test_index);
    printf("Quantum state at index %llu: %.6f + %.6fi\n", test_index, creal(read_state), cimag(read_state));

    double error = qmem_error_rate(qmem, test_index);
    printf("Current error rate at index %llu: %.10e\n", test_index, error);

    qmem_free(qmem);
    return 0;
}

