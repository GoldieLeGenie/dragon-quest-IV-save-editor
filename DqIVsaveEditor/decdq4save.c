#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 0x3660LL  
#define CHECKSUM_OFFSET 8    

void backup_crypt(unsigned char *data, int size) {
    unsigned int v7 = 0xBB6E07FB; 
    for (int i = 0; i < size; i++) {
        unsigned int v14 = v7 ^ (v7 << 13) ^ ((v7 ^ (v7 << 13)) >> 17);
        v7 = v14 ^ (32 * v14);
        data[i] ^= v7 & 0xFF; 
    }
}

void encrypt_file(const char *input_file) {
    FILE *file_in = fopen(input_file, "rb");
    if (!file_in) {
        perror("Error opening input file");
        return;
    }

    fseek(file_in, 0, SEEK_END);
    long file_size = ftell(file_in);
    if (file_size <= 0) {
        perror("Error determining file size");
        fclose(file_in);
        return;
    }
    fseek(file_in, 0, SEEK_SET);

    unsigned char *buffer = (unsigned char *)malloc(file_size);
    if (!buffer) {
        perror("Memory allocation error");
        fclose(file_in);
        return;
    }

    if (fread(buffer, 1, file_size, file_in) != file_size) {
        perror("Error reading file");
        free(buffer);
        fclose(file_in);
        return;
    }
    fclose(file_in);

    backup_crypt(buffer, file_size);

    FILE *file_out = fopen(input_file, "wb");
    if (!file_out) {
        perror("Error opening output file");
        free(buffer);
        return;
    }

    if (fwrite(buffer, 1, file_size, file_out) != file_size) {
        perror("Error writing to file");
    }

    fclose(file_out);
    free(buffer);

    printf("Done : %s\n", input_file);
}

void decrypt_file(const char *input_file) {
    encrypt_file(input_file); 
}

void checksum(const char *file_path) {
    FILE *file = fopen(file_path, "rb+");
    if (!file) {
        perror("Error opening file");
        return;
    }

    unsigned char data[DATA_SIZE] = {0};

    if (fread(data, 1, DATA_SIZE, file) != DATA_SIZE) {
        perror("Error reading file");
        fclose(file);
        return;
    }

    memset(&data[CHECKSUM_OFFSET], 0, 4);

    unsigned int checksum = 0;
    for (int i = 0; i < DATA_SIZE; i++) {
        checksum += data[i];
    }
    checksum &= 0xFFFFFFFF;

    data[CHECKSUM_OFFSET] = (checksum & 0xFF);
    data[CHECKSUM_OFFSET + 1] = (checksum >> 8) & 0xFF;
    data[CHECKSUM_OFFSET + 2] = (checksum >> 16) & 0xFF;
    data[CHECKSUM_OFFSET + 3] = (checksum >> 24) & 0xFF;

    fseek(file, 0, SEEK_SET);
    if (fwrite(data, 1, DATA_SIZE, file) != DATA_SIZE) {
        perror("Error writing file");
    }

    fclose(file);
    printf("Checksum replaced!\n");
}

int main() {
    const char *fileinput = "data1.dat"; 
    decrypt_file(fileinput);
    checksum(fileinput); 
    encrypt_file(fileinput);

    return 0;
}
