//
//  openafc_sf.c
//  openafc_sf
//
//  Created by cal0x on 14/01/2014.
//  Copyright (c) 2014 Louis Kremer. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include <libimobiledevice/afc.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/installation_proxy.h>

#define KNRM  "\x1B[0m"
#define KGRN  "\x1B[32m"
#define KRED  "\x1B[31m"
#define KWHT  "\x1B[37m"

uint64_t gFh = 0;
unsigned int cb = 0;
unsigned int installError = 0;
unsigned int installing = 1;

idevice_t idevice = NULL;
afc_client_t afc_client = NULL;
lockdownd_client_t lockdownd_client = NULL;

int afc_send_file(afc_client_t afc, const char* local, const char* remote) {
	FILE* fd = NULL;
	uint64_t fh = 0;
	afc_error_t err = 0;
	unsigned int got = 0;
	unsigned int gave = 0;
	unsigned char buffer[0x800];
    
	fd = fopen(local, "rb");
	if (fd != NULL ) {
		err = afc_file_open(afc, remote, AFC_FOPEN_WR, &fh);
		if (err == AFC_E_SUCCESS) {
            
			while (!feof(fd)) {
				memset(buffer, '\0', sizeof(buffer));
				got = fread(buffer, 1, sizeof(buffer), fd);
				if (got > 0) {
					afc_file_write(afc, fh, (const char*) buffer, got, &gave);
					if (gave != got) {
						printf("Error!!\n");
						break;
					}
				}
			}
            
			afc_file_close(afc, fh);
		}
		fclose(fd);
	} else
		return -1;
	return 0;
}

int main(int argc, char *argv[]) {
    idevice_error_t idevice_error = 0;
	idevice_error = idevice_new(&idevice, NULL);
	if (idevice_error != IDEVICE_E_SUCCESS) {
		return -1;
	}
    lockdownd_error_t lockdown_error = 0;
	lockdown_error = lockdownd_client_new_with_handshake(idevice, &lockdownd_client, "openafc");
	if (lockdown_error != LOCKDOWN_E_SUCCESS) {
		return -1;
	}
	lockdownd_service_descriptor_t port = NULL;
	lockdown_error = lockdownd_start_service(lockdownd_client, "com.apple.afc", &port);
	if (lockdown_error != LOCKDOWN_E_SUCCESS) {
		return -1;
	}
    afc_error_t afc_do_it = 0;
	afc_do_it = afc_client_new(idevice, port, &afc_client);
	if (afc_do_it != AFC_E_SUCCESS) {
		lockdownd_client_free(lockdownd_client);
		idevice_free(idevice);
		return -1;
	}
	lockdownd_client_free(lockdownd_client);
	lockdownd_client = NULL;
    
    
    afc_do_it = afc_send_file(afc_client, "example.tar", "example.tar");
 // afc_do_it = afc_send_file(afc_client, "files/picture.png", "Downloads/new_name.png");
    if (afc_do_it != AFC_E_SUCCESS) {
        printf("%sError with sending file!\n", KRED);
        printf("%s", KNRM);
    }
        else {
            printf("%sSuccessfully sended file\n", KGRN);
            printf("%s", KNRM);
        }
}
