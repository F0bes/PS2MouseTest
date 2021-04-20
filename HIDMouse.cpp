/*
MIT License

Copyright (c) 2021 Ty Lamontagne (Fobes)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <kernel.h>

#include <cstdio>
#include <unistd.h>

#include <sifrpc.h>
#include <sifdma.h>
#include <sifcmd.h>

#include <sbv_patches.h>
#include <iopcontrol.h>
#include <loadfile.h>

#include <libmouse.h>

#include <gsKit.h>
#include <dmaKit.h>
#include <gsToolkit.h>

extern unsigned int size_iomanX;
extern unsigned char iomanX[];

extern unsigned int size_usbd;
extern unsigned char usbd[];

extern unsigned int size_ps2mouse;
extern unsigned char ps2mouse[];

gsGlobal *g_gsInst;

 u64 gs_BackgroundColors[] =
{
    GS_SETREG_RGBAQ(255,0,0,0,0),
    GS_SETREG_RGBAQ(0,255,0,0,0),
    GS_SETREG_RGBAQ(0,0,255,0,0),
    GS_SETREG_RGBAQ(255,255,0,0,0),
    GS_SETREG_RGBAQ(255,0,255,0,0),
    GS_SETREG_RGBAQ(255,255,0,0,0),
    GS_SETREG_RGBAQ(255,128,0,0,0),
};
int gs_BackgroundColorsSize = sizeof(gs_BackgroundColors) / sizeof(u64) - 1;
int gs_BackgroundColorsCur = 0;


int main(void)
{
    printf("HIDMouse test starting\n");
    printf("****Initializing PS2 mouse****\n");

    SifInitRpc(0);
	sbv_patch_enable_lmb(); // Wasted a couple hours only to find out this needs to be called 
    
    int ret = SifExecModuleBuffer(iomanX,size_iomanX,0,NULL,NULL);
    printf("iomanx module id %d\n",ret);
    ret = SifExecModuleBuffer(usbd,size_usbd,0,NULL,NULL);
    printf("usbd module id %d\n",ret);
    ret = SifExecModuleBuffer(ps2mouse,size_ps2mouse,0,NULL,NULL);
    printf("ps2mouse module id %d\n",ret);

    ret = PS2MouseInit();

	if(ret < 0)
	{
		printf("Couldn't initialise ps2mouse (%d)",ret);
		SleepThread();
	}

    PS2MouseSetReadMode(PS2MOUSE_READMODE_ABS);
    PS2MouseSetPosition(0,0);

    printf("****Initializing GSKit****\n");
    
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
		    D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    
	dmaKit_chan_init(DMA_CHANNEL_GIF);

    g_gsInst = gsKit_init_global();
    
    gsKit_init_screen(g_gsInst);
    gsKit_set_test(g_gsInst, GS_ZTEST_OFF);
    gsKit_mode_switch(g_gsInst, GS_ONESHOT);
                                                            // Gets rid of the non-const warning LOL
    GSFONT *gsFont = gsKit_init_font(GSKIT_FTYPE_BMP_DAT, const_cast<char*>("host:dejavu.bmp"));

    gsKit_font_upload(g_gsInst, gsFont);

    printf("****Initializing GSKit and PS2Mouse done****\n");

    PS2MouseData mouseData;
    while(1)
    {
        PS2MouseRead(&mouseData);
        int x = mouseData.x * 0.5,y = mouseData.y * 0.5; // Divide by two just because pcsx2 mouse input is kind of weird

        gsKit_clear(g_gsInst,gs_BackgroundColors[gs_BackgroundColorsCur]);

        if(mouseData.buttons & PS2MOUSE_BTN1)
        {
            gsKit_prim_triangle(g_gsInst,x,y,x + 20,y,x, y - 20,1,GS_SETREG_RGBAQ(255,255,255,0,0));     
        }
        else
        {
            gsKit_prim_sprite(g_gsInst,x,y,x + 5,y + 5,1,GS_SETREG_RGBAQ(255,255,255,0,0));     
        }

        if(mouseData.buttons & PS2MOUSE_BTN2)
        {
            if(gs_BackgroundColorsCur++ == gs_BackgroundColorsSize)
            {
                gs_BackgroundColorsCur = 0;
            }
            for(int a = 0; a /2 < 5000000; a++) // There's really no simple way to sleep
                    asm("nop");            
        }

        if(mouseData.buttons & PS2MOUSE_BTN3)
        {
            PS2MouseSetPosition(0,0);
        }

        
        gsKit_font_print_scaled(g_gsInst, gsFont,0, g_gsInst->Height - gsFont->CharHeight * 2, 1, 2, GS_SETREG_RGBAQ(255,255,255,0,0), "F0bes");

        gsKit_queue_exec(g_gsInst);
		gsKit_sync_flip(g_gsInst);
    }
    SleepThread();
}
