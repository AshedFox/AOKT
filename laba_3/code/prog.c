#include <stdio.h>
#include <sys/io.h>

#include <errno.h>
#include <stdlib.h>

#include <string.h>
#include <stdbool.h>
#include <values.h>
#include "pci.h"

//-------------------------END INCLUDES------------------------


#define MAX_BUS 256
#define MAX_DEVICE 32
#define MAX_FUNCTIONS 8

#define ID_REGISTER 0

#define DEVICEID_SHIFT 16
#define BUS_SHIFT 16
#define DEVICE_SHIFT 11
#define FUNCTION_SHIFT 8
#define REGISTER_SHIFT 2

#define CONTROL_PORT 0x0CF8
#define DATA_PORT 0x0CFC

//-------------------------END DEFINES------------------------




void PrintInfo(int bus, int device, int function);
bool IfBridge(int bus,int device, int function);
long readRegister(int bus, int device, int function, int reg);
void outputGeneralData(int bus, int device, int function, int regData);
char *getDeviceName(int vendorID, int deviceID);
char *getVendorName(int vendorID);
void outputClassCodeData(long regData);
void outputBaseIORegData(int bus, int device, int function);
void outputInterruptPinData(long regData);


FILE *out;

//------TASK 1------
void outputClassCodeData(long regData){
    fputs("------TASK 1------\n", out);
    puts("------TASK 1------\n");

    unsigned baseClass = (regData >> 24) & 0xFF;
    unsigned subClass = (regData >> 16) & 0xFF;
    unsigned specRegLevelProgInterface = (regData >> 8) & 0xFF;

    fprintf(out, "Base class %Xh\n%Xh  %Xh\n", baseClass, subClass, specRegLevelProgInterface);
    printf("Base class %Xh\n%Xh  %Xh\n", baseClass, subClass, specRegLevelProgInterface);
}

//------TASK 3------
void outputBaseIORegData(int bus, int device, int function){
    fprintf(out, "------TASK 3------\n");
    printf("------TASK 3------\n");
    for (int i = 0; i < 6; i++){
        unsigned int regData = readRegister(bus, device, function, 4 + i);
        if (regData) {
            if (regData & 1){
                printf("BAR %d is BASE I/O REGISTER\n", i+1);
                fprintf(out, "BAR %d is BASE I/O REGISTER\n", i+1);
                regData = regData >> 2;
                printf("\tBASE ADDRESS: %Xh\n", regData);
                fprintf(out, "\tBASE ADDRESS: %Xh\n", regData);
            }
            else {
                printf("BAR %d is BASE MEMORY REGISTER\n", i+1);
                fprintf(out, "BAR %d is BASE MEMORY REGISTER\n", i+1);
            }
        }
        else {
            printf("BAR %d is UNUSED\n", i+1);
            fprintf(out, "BAR %d is UNUSED\n", i+1);
        }
    }
}

//------TASK 6------
void outputInterruptPinData(long regData){
    fprintf(out, "------TASK 6------\n");
    printf("------TASK 6------\n");

   int interruptPin = (regData >> 8) & 0xFF;
   char *interruptPinData;

   switch (interruptPin) {
       case 0:
           interruptPinData = "Not used";
           break;
       case 1:
           interruptPinData = "INTA#";
           break;
       case 2:
           interruptPinData = "INTB#";
           break;
       case 3:
           interruptPinData = "INTC#";
           break;
       case 4:
           interruptPinData = "INTD#";
           break;
       default:
           interruptPinData = "Invalid pin number";
           break;
   }

   fprintf(out, "Interrupt pin: %s\n", interruptPinData);
   printf( "Interrupt pin: %s\n", interruptPinData);
}


//------GENERAL------

char *getVendorName(int vendorID) {
int i;
    for (i = 0; i < PCI_VENTABLE_LEN; i++) {
        if (PciVenTable[i].VendorId == vendorID) {
            return PciVenTable[i].VendorName;
        }
    }
    return NULL;
}

char *getDeviceName(int vendorID, int deviceID) {
int i;
    for ( i = 0; i < PCI_DEVTABLE_LEN; i++) {
        if (PciDevTable[i].VendorId == vendorID && PciDevTable[i].DeviceId == deviceID) {
            return PciDevTable[i].DeviceName;
        }
    }
    return NULL;
}

void outputVendorData(int vendorID)
{
    char *vendorName = getVendorName(vendorID);
    fprintf(out, "Vendor ID: %04d, %s\n", vendorID, vendorName ? vendorName : "unknown vendor");
    printf( "Vendor ID: %04d, %s\n", vendorID, vendorName ? vendorName : "Unknown vendor");
}

void outputDeviceData(int vendorID, int deviceID)
{
    char *deviceName = getDeviceName(vendorID, deviceID);
    fprintf(out, "Device ID: %04d, %s\n", deviceID, deviceName ? deviceName : "unknown device");
	printf( "Device ID: %04d, %s\n", deviceID, deviceName ? deviceName : "Unknown device");
}

void outputGeneralData(int bus, int device, int function, int regData){
    fprintf(out, "%x:%x:%x\n", bus, device, function);
    printf( "%x:%x:%x\n", bus, device, function);
    int deviceID = regData >> DEVICEID_SHIFT;
    int vendorID = regData & 0xFFFF;
    outputVendorData(vendorID);
    outputDeviceData(vendorID, deviceID);
}



long readRegister(int bus, int device, int function, int reg) {
    long  configRegAddress = (1 << 31) | (bus << BUS_SHIFT) | (device << DEVICE_SHIFT) |(function << FUNCTION_SHIFT) | (reg << REGISTER_SHIFT);
    outl(configRegAddress, CONTROL_PORT);
    return inl(DATA_PORT);

	return 0;
}

bool IfBridge(int bus,int device, int function){
    long htypeRegData = readRegister(bus, device, function, 3);
    return ((htypeRegData >> 16) & 0xFF) & 1;
}


void PrintInfo(int bus, int device, int function) {
   long idRegData = readRegister(bus, device, function, ID_REGISTER);

   if (idRegData != 0xFFFFFFFF) { // if there is a device 
        outputGeneralData(bus, device, function, idRegData);

        if (IfBridge(bus, device, function)) {
            fprintf(out, "\nIs bridge\n\n");
            printf("\nIs bridge\n\n");          
        } else {
            fprintf(out, "\nNot a bridge\n\n");
            printf("\nNot a bridge\n\n");
            outputClassCodeData(readRegister(bus,device,function, 2));
            outputBaseIORegData(bus, device, function);
            outputInterruptPinData(readRegister(bus, device, function, 15));   
        }
        printf("---------------------------------------------------\n");
        fprintf(out, "---------------------------------------------------\n");
    }
}


int main() { 
	
    if (iopl(3)) {   
        printf("I/O Privilege level change error: %s\nTry running under ROOT user\n", strerror(errno));
        return 2;
    }

    int bus; 
    int device;
    int func;
    out = fopen("output.txt", "w");

    for ( bus = 0; bus < MAX_BUS; bus++){
        for (device = 0; device < MAX_DEVICE; device++){
            for ( func = 0; func < MAX_FUNCTIONS; func++){
                PrintInfo(bus,device,func);
            }
        }
    }

    fclose(out);
    return 0;
}

