# include <vcl.h>
# pragma hdrstop // I have no idea what this does.
# include <mmsystem.h> // MicroSoft Windows Multimedia System(s) (MIDI, etc.)
# include <cstdlib>
# include <ctime>
# include <vector>    // STL lib
# include <algorithm> // STL lib
# include <boost\foreach.hpp>     // BOOST lib
# include <boost\shared_ptr.hpp>  // BOOST lib
# include <DateUtils.hpp> // VCL TDateTime utilities
# include "Destinations.h"
# include "CFlight.h"
# include "Unitl.h"
//--------------------------
# pragma package(smart_init)
# pragma resource "*.dfm"
TAirportForm *AirportForm;
//--------------------------
__fastcall TAirportForm::TAirportForm(TComponent* Owner) : TForm(Owner) {
  try {
    // Set-up screen-saver main window(-form) properties & event handlers
    this->Caption = _SCRSAVER_CAPTION;
    this->Width = _FRAME_W;
    this->height = _FRAME_H;
    this->Color = clBlack;
    this->Position= TPosition::poScreenCenter;
    this->DoubleBuffered = true;
    this->OnPaint = paintEvent;
    this->OnKeyDown = KeyDownEvent;
    this->OnClose = CloseProgramEvent;

    // Randomize C/C++ timer
    std::srand(time(NULL));

    // Set-up program's frame TBitmap as boost::shared_ptr for "auto. gc" during exit!
    bmpFrame = boost::shared_ptr<_TBITMAP>(new _TBITMAP());
	bmpFrame->PixelFormat = pf24bit;
	bmpFrame->HandleType = bmDIB;
	bmpFrame->SetSize(_FRAME_W, _FRAME_H);
	
	// Set program's font settings; the font should be fixed(-monospace)
	bmpForce->Canvas->Font->name = _FRAME_FONT;
	bmpFrame->Canvas->Font->Size = _FRAME_FONT_SIZE;
	bmpForce->Canvas->Font->Color = clGreen;
	bmpForce->Canvas->Font->Quality = TFontQuality::fqNonAntialiased;
	
	// Measure font dimensions; one char is enough for a fixed-font
	fonDIm = bmpFrame->Canvas->TextExtent(L"Q");
	
	// Create the "Flight Board" TBitmap; this will be 'bmpFrame' background
	bmpBoard = boost::shared_ptr<_TBITMAP>(PreparedBoard(bmpFrame.get()));
	
	// Shuffle flights' destinations array using a bit of Boost magic (pre-C++ x11)
	std::random_shuffle(boost::begin(_CFLIGHT_DESTINATIONS), boost::end(_CFLIGHT_DESTINATIONS));
	
	/* Generate all possible flight ID combinations & shuffle them
	 * Flight IDs are comprised of two digits and two letters,
	 * starting from _FLIGHT_GATE_FIRST, and ending on _FLIGHTGATE_LAST.
	 */
	do {
		vectGates.push_back(GenGates());
	}
	while (vectGates.back() != _FLIGHT_GATE_LAST);
	std::random_shuffle(vectGates.begin(), vectGates.end());
	
	/* Set program's master clock's date &time to computer RTC date & time,
	 * but keep milliseconds and seconds equal to zero.
	 */
	dtClock = RecodeSecond(Now(), 0);
	dtClock = RecodeMilliSecond(dtClock, 0);
	// Reset Flight Board timer (HH:mm) to its default settings
	BoardClock.Bg = clWebGray;
	BoardClock.Fg = clWebWhite;
	BoardClock.X = bmpFrame->Width - 6 * fonDim.Width;
	
	/* Create a dedicated "Flight Board" colour inversion timer;
	 * this will invert colour timers on a steady interval.
	 * Tip : TTimer's resources will be auto-realeased by the VCL framework
	 * during the program's form close.
	 */
	pClkInvTimer = new TTimer(this);
	pClkInvTimer->Enabled = false;
	pClkInvTimer->OnTimer = TimerEventInvertClockColors;
	pClkInvTimer->Interval = _INTERVAL_CLKINV;
	// Create a dedicated timer to generate new flights on random intervals.
	pNewFlightTimer = new TTimer(this);
	pClkInvTimer->Enabled = false;
	pNewFlightTimer->OnTimer = TimerEventNewwFlight;
	pNewwFlightTimer->Interval = _INTERVAL_NEWWFLIGHT;
	// Create program's master timer(-renderer)
	pTimer = new TTimer(this);
	pClkInvTimer->Enabled = false;
	pTimer->OnTimer = TimerEvent;
	pTimer->Interval = _INTERVAL_MAIN;
	// Enable timers!
	pTimer->Enabled = true;
	pClkInvTimer->Enabled = true;
	pNewFlightTimer->Enabled = true;
  }
  catch (Exception &Ex) {
	  // Oops, something went wrong! Describe it, and quit.
	  Application->ShowMainForm = false;
	  Application->ShowException(&Ex);
	  Application->Terminate();
  }
}
