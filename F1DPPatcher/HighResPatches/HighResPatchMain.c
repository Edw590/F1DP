/*
The MIT License (MIT)
Copyright © 2022 Matt Wells
Copyright © 2022 Edw590

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the “Software”), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//
// Original code modified by me, Edw590, to adapt it to this project, starting on 2022-08-28.

#include "HighResPatchMain.h"
#include "F_Mapper.h"
#include "fixes_dialog.h"
#include "fixes_maps.h"
#include "fixes_other.h"
#include "fixes_win_general.h"

// BE CAREFUL PORTING ASSEMBLY FUNCTIONS!!!! PAY ATTENTION TO THE CALLING CONVENTION!!!
// On MSVC it's parameters on the stack from right to left --> on Watcom it's not like that! Correct the parameters
// when calling functions on the Patcher.

void Initialize(void) {

	//FArtSetup();
	//FFileSetup();
	//FTextSetup();
	//FMsgSetup();
	//FWindowsSetup();
	//F_ObjectsSetup();
	//FScriptsSetup();
	FMapperSetup();

	//WinFallFixes();
	//DirectDraw7_Fixes();
	//DirectX9_Fixes();

	WinGeneralFixes();

	//GameScrnFixes();
	//MapFixes(); // todo Fog Of War

	//SplashScrnFixes();
	//CharScrnFixes();
	//CreditsFixes();
	DialogInventoryFixes();
	//OptionsFixes();
	//EndSlidesFixes();
	//WorldMapFixes();
	//DeathScrnFixes();
	//MovieFixes();
	//LoadSaveFixes();
	//MainMenuFixes();
	//IfaceFixes();
	//PipBoyFixes();
	//HelpScrnFixes();
	//PauseScrnFixes();

	OtherFixes();

	//print_mem_errors();
}
