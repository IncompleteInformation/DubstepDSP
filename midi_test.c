#include "midi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main (void)
{
    midi_init();

    // Play a chord
    midi_write(Pm_Message(0x90, 60, 100));
    midi_write(Pm_Message(0x90, 64, 100));
    midi_write(Pm_Message(0x90, 67, 100));
    midi_flush();
    
    printf("num_devices: %i\n",Pm_CountDevices());
    PmDeviceID id = Pm_GetDefaultOutputDeviceID();
    const PmDeviceInfo* device = Pm_GetDeviceInfo(id);
    printf("%s\n", device->name);
    
    
    char setchar = '.';
	int channel = 0;
	while (setchar!='\e'){
		scanf(" %c", &setchar);
		//printf("%c\n", setchar);
		switch (setchar){
			case 'z': channel = 0xB0; break;
			case 'x': channel = 0xB1; break;
			case 'c': channel = 0xB2; break;
			case 'v': channel = 0xB3; break;
                // 'q': Pm_WriteShort(midi, 0, Pm_Message(channel, 10, 0)); //button1
                // 'w': Pm_WriteShort(midi, 0, Pm_Message(channel, 11, 0)); //button2
                // 'e': Pm_WriteShort(midi, 0, Pm_Message(channel, 12, 0)); //button3
                // 'r': Pm_WriteShort(midi, 0, Pm_Message(channel, 13, 0)); //button4
			case '1': midi_write(Pm_Message(channel, 0, 0)); midi_flush(); break;
			case '2': midi_write(Pm_Message(channel, 1, 0)); midi_flush(); break;//hand angle
                // '7': Pm_WriteShort(midi, 0, Pm_Message(channel, 7, 0));
                // '8': Pm_WriteShort(midi, 0, Pm_Message(channel, 8, 0));
                // '9': Pm_WriteShort(midi, 0, Pm_Message(channel, 9, 0));
                // '0': Pm_WriteShort(midi, 0, Pm_Message(channel, 10, 0));
			default:
                break;
		}
	}

    midi_cleanup();
    return 0;
}