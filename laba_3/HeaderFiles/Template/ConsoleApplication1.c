#include <stdio.h>
#include "pci_c_header.h"

int main() {
	printf("%s", PciVenTable[10].VenFull);
}
