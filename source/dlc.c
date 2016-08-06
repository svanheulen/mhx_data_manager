/*
Copyright 2016 Seth VanHeulen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <3ds.h>
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <string.h>
#include "ui.h"

static Handle frdu_handle = 0;

Result frduSetClientSdkVersion(int version) {
    if (frdu_handle == 0)
        return 0xD8A0C7F8;
    u32* command_buffer = getThreadCommandBuffer();
    command_buffer[0] = 0x320042;
    command_buffer[1] = version;
    command_buffer[2] = 0x20;
    Result r = svcSendSyncRequest(frdu_handle);
    if (R_FAILED(r))
        return r;
    return command_buffer[1];
}

int frduGetMyFriendKey() {
    if (frdu_handle == 0)
        return 0;
    u32* command_buffer = getThreadCommandBuffer();
    command_buffer[0] = 0x50000;
    Result r = svcSendSyncRequest(frdu_handle);
    if (R_FAILED(r))
        return 0;
    if (command_buffer[1] & 0x80000000)
        return 0;
    return command_buffer[2];
}

Result frduRequestServiceLocator(Handle* event, int title_id, const char* unknown3, const char* unknown4, char unknown5, char unknown6) {
    if (frdu_handle == 0)
        return 0xD8A0C7F8;
    if (event == NULL || unknown3 == NULL || unknown4 == NULL)
        return 0xE0E0C7F6;
    if (*event == 0)
        return 0xE0E0C7F7;
    u32* command_buffer = getThreadCommandBuffer();
    command_buffer[0] = 0x2A0204;
    command_buffer[1] = title_id;
    command_buffer[2] = ((int*) unknown3)[0];
    command_buffer[3] = ((int*) unknown3)[1];
    command_buffer[4] = unknown3[8];
    command_buffer[5] = ((int*) unknown4)[0];
    command_buffer[6] = unknown4[4];
    command_buffer[7] = unknown5;
    command_buffer[8] = unknown6;
    command_buffer[9] = 0x20;
    command_buffer[11] = 0;
    command_buffer[12] = *event;
    Result r = svcSendSyncRequest(frdu_handle);
    if (R_FAILED(r))
        return r;
    return command_buffer[1];
}

Result frduGetServiceLocatorData(void* buffer) {
    if (frdu_handle == 0)
        return 0xD8A0C7F8;
    if (buffer == NULL)
        return 0xE0E0C7F6;
    u32* command_buffer = getThreadCommandBuffer();
    command_buffer[0] = 0x2B0000;
    u32* static_buffers = getThreadStaticBuffers();
    u32 backup[2];
    backup[0] = static_buffers[0];
    static_buffers[0] = 0x660002;
    backup[1] = static_buffers[1];
    static_buffers[1] = (u32) buffer;
    Result r = svcSendSyncRequest(frdu_handle);
    static_buffers[0] = backup[0];
    static_buffers[1] = backup[1];
    if (R_FAILED(r))
        return r;
    return command_buffer[1];
}

int initialize_post_data(char* post_data, char* common_key) {
    ui_info_add("Requesting auth token ... ");
    Handle event;
    if (svcCreateEvent(&event, 0) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to create event");
        return 0;
    }
    if (srvGetServiceHandle(&frdu_handle, "frd:u") != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable initialize friend service");
        svcCloseHandle(event);
        return 0;
    }
    int ret = 0;
    char buffer[0x198] = {0};
    int friend_key = frduGetMyFriendKey();
    if (friend_key == 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to get friend key");
    } else if (frduSetClientSdkVersion(0xB0101C8) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to set SDK version");
    } else if (svcClearEvent(event) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to clear event");
    } else if (frduRequestServiceLocator(&event, 0x155400, "ffa2340f", "0000", 0, 0) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to request service locator");
    } else if (svcWaitSynchronization(event, -1) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unanle to wait for event");
    } else if (frduGetServiceLocatorData(buffer) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to get service locator data");
    } else {
        post_data[1] = strlen(buffer + 0x88);
        strncpy(post_data + 2, buffer + 0x88, 0x100);
        post_data[post_data[1] + 5] = 1;
        s64 client_nonce = svcGetSystemTick();
        common_key[0] = common_key[12] = ((char*) &friend_key)[3];
        common_key[1] = common_key[13] = ((char*) &friend_key)[2];
        common_key[2] = common_key[14] = ((char*) &friend_key)[1];
        common_key[3] = common_key[15] = ((char*) &friend_key)[0];
        common_key[4] = post_data[post_data[1] + 17] = (client_nonce >> 24) & 0xff;
        common_key[5] = post_data[post_data[1] + 18] = (client_nonce >> 16) & 0xff;
        common_key[6] = post_data[post_data[1] + 19] = (client_nonce >> 8) & 0xff;
        common_key[7] = post_data[post_data[1] + 20] = client_nonce & 0xff;
        ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
        ret = 1;
    }
    svcCloseHandle(event);
    svcCloseHandle(frdu_handle);
    return ret;
}

int request_key(char* post_data, char* common_key, char version, char* url, char* response, int response_size) {
    ui_info_add("  Sending request ... ");
    post_data[0] = version;
    unsigned char hmac_key[32];
    mbedtls_sha256((unsigned char*) common_key, 8, hmac_key, 0);
    mbedtls_md_hmac(mbedtls_md_info_from_string("SHA256"), hmac_key, 32, (unsigned char*) post_data, post_data[1] + 21, (unsigned char*) post_data + post_data[1] + 21);
    if (httpcInit(0x1000) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to initialize HTTP service");
        return 0;
    }
    httpcContext context;
    if (httpcOpenContext(&context, HTTPC_METHOD_POST, url, 1) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to open HTTP context");
        httpcExit();
        return 0;
    }
    int ret = 0;
    u32 status = 0;
    if (httpcSetSSLOpt(&context, SSLCOPT_DisableVerify) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to set SSL option");
    } else if (httpcAddRequestHeaderField(&context, "User-Agent", "Capcom Browser Services for MonsterHunter_X") != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to add HTTP header");
    } else if (httpcAddPostDataRaw(&context, (u32*) post_data, post_data[1] + 53) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to add POST data");
    } else if (httpcBeginRequest(&context) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to begin HTTP request");
    } if (httpcGetResponseStatusCode(&context, &status) != 0 || status != 200) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to get HTTP status code or recieved\nerror status code from server");
    } else {
        Result res = 0;
        int response_len = 0;
        u32 read = 0;
        do {
            res = httpcDownloadData(&context, (u8*) response + response_len, response_size - response_len, &read);
            response_len += read;
        } while (res == HTTPC_RESULTCODE_DOWNLOADPENDING);
        if (res != 0) {
            ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
            ui_pause("Error: Unable to read response");
        } else {
            ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
            ret = response_len;
        }
    }
    httpcCloseContext(&context);
    httpcExit();
    return ret;
}

int verify_response(char* response, int response_len, char* common_key) {
    ui_info_add("  Verifying response ... ");
    common_key[8] = response[response_len - 36];
    common_key[9] = response[response_len - 35];
    common_key[10] = response[response_len - 34];
    common_key[11] = response[response_len - 33];
    unsigned char md[32];
    mbedtls_sha256((unsigned char*) common_key, 12, md, 0);
    mbedtls_md_hmac(mbedtls_md_info_from_string("SHA256"), md, 32, (unsigned char*) response, response_len - 32, md);
    if (memcmp(md, response + response_len - 32, 32) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to verify reponse HMAC");
        return 0;
    }
    ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
    return 1;
}

void log_response(char* response, int response_len, char* common_key, char version, char* log_file) {
    ui_info_add("  Logging response ... ");
    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_dec(&aes_ctx, (unsigned char*) common_key, 128);
    int key_len = (response[0] << 8) + response[1];
    unsigned char iv[16] = {0};
    char buffer[0x200] = {0};
    mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, key_len, iv, (unsigned char*) response + 2, (unsigned char*) buffer);
    char* end = strchr(buffer, 8);
    if (end != NULL)
        *end = 0;
    FILE* log = fopen(log_file, "w");
    if (log == NULL) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to open log file");
        return;
    }
    if (fprintf(log, "Blowfish Key = %s\n", buffer) < 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to write to log file");
        return;
    }
    if (version == 3) {
        unsigned int pubkey_b64_len = 0;
        int pubkey_len = (response[key_len + 2] << 8) + response[key_len + 3];
        mbedtls_base64_encode((unsigned char*) buffer, 0x200, &pubkey_b64_len, (unsigned char*) response + key_len + 4, pubkey_len);
        buffer[pubkey_b64_len] = 0;
        if (fprintf(log, "RSA Public Key = %s\n", buffer) < 0) {
            ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
            ui_pause("Error: Unable to write to log file");
            return;
        }
        int url_len = (response[key_len + pubkey_len + 4] << 8) + response[key_len + pubkey_len + 5];
        memcpy(buffer, response + key_len + pubkey_len + 6, url_len);
        buffer[url_len] = 0;
        if (fprintf(log, "DLC Server URL = %s\n", buffer) < 0) {
            ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
            ui_pause("Error: Unable to write to log file");
            return;
        }
    } else {
        int url_len = (response[key_len + 2] << 8) + response[key_len + 3];
        memcpy(buffer, response + key_len + 4, url_len);
        buffer[url_len] = 0;
        if (fprintf(log, "DLC Server URL = %s\n", buffer) < 0) {
            ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
            ui_pause("Error: Unable to write to log file");
            return;
        }
    }
    fclose(log);
    ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
}

void get_encryption_keys() {
    ui_info_clear();
    char post_data[0x135];
    char common_key[16];
    memset(post_data, 0, 0x135);
    if (initialize_post_data(post_data, common_key)) {
        char response[0x200];
        ui_info_add("Getting JPN key ... \n");
        int response_len = request_key(post_data, common_key, 2, "https://spector.capcom.co.jp/SSL/3ds/mhx/login_jp.cgi", response, 0x200);
        if (response_len != 0 && verify_response(response, response_len, common_key))
            log_response(response, response_len, common_key, 2, "key/jpn.log");
        ui_info_add("Complete.\nGetting EUR key ... \n");
        response_len = request_key(post_data, common_key, 3, "https://spector.capcom.co.jp/SSL/3ds/mhx/login_eu.cgi", response, 0x200);
        if (response_len != 0 && verify_response(response, response_len, common_key))
            log_response(response, response_len, common_key, 3, "key/eur.log");
        ui_info_add("Complete.\nGetting USA key ... \n");
        response_len = request_key(post_data, common_key, 3, "https://spector.capcom.co.jp/SSL/3ds/mhx/login_us.cgi", response, 0x200);
        if (response_len != 0 && verify_response(response, response_len, common_key))
            log_response(response, response_len, common_key, 3, "key/usa.log");
        ui_info_add("Complete.\n");
    }
}

