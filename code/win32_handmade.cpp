
#include <windows.h>
#include <stdint.h>

#define internal_function static
#define local_persist static
#define global_variable static  // global statics are automatically initialized to 0

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal_function void
Win32RenderGradient(int BlueOffset, int GreenOffset)
{
	int pitch = BitmapWidth*BytesPerPixel;
	uint8 *row = (uint8*)BitmapMemory;
	for(int Y = 0; Y < BitmapHeight; ++Y)
	{
		uint32 *pixel = (uint32*)row;

		for(int X = 0; X < BitmapWidth; ++X)
		{

			uint8 blue = (X + BlueOffset);
			uint8 green = (Y + GreenOffset);

			*pixel++ = ((green << 8) | blue);

			// Pixel in mem: B G R XX 
			// *Pixel = (uint8)(X+XOffset);
			// ++Pixel;
			// *Pixel = (uint8)(Y+YOffset);
			// ++Pixel;
			// *Pixel = 0;
			// ++Pixel;
			// *Pixel = 0;
			// ++Pixel;
		}

		row += pitch;
	}

}

internal_function void
Win32ResizeDIBSection(int Width, int Height)
{

	/// free the global handle before creating a new bitmap

	if(BitmapMemory)
	{
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	}


	BitmapWidth = Width;
	BitmapHeight = Height;

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	// no more create dc as we dont need it. strechdibits takes a pointer to mem

	/// REFERENCE VirtualAlloc
	/// http://msdn.microsoft.com/en-us/library/windows/desktop/aa366887%28v=vs.85%29.aspx


	int bitmap_memory_size = (Width * Height)*BytesPerPixel;
	BitmapMemory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
}


internal_function void
Win32UpdateWindow(HDC DeviceContext, RECT WindowRect, int X, int Y, int Width, int Height)
{
	/// REFERENCE BitBlt
	/// http://msdn.microsoft.com/en-us/library/windows/desktop/dd183370(v=vs.85).aspx

	/// REFERENCE StretchDIBits
	/// http://msdn.microsoft.com/en-us/library/windows/desktop/dd145121(v=vs.85).aspx

	int WindowWidth = WindowRect.right - WindowRect.left;
	int WindowHeight = WindowRect.bottom - WindowRect.top;

	StretchDIBits(
		DeviceContext,
		// X, Y, Width, Height,
		// X, Y, Width, Height,
		0, 0, BitmapWidth, BitmapHeight,
		0, 0, WindowWidth, WindowHeight,
		BitmapMemory,
		&BitmapInfo,
		DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK
MainProcCallback(HWND Window,
				 UINT Message,
				 WPARAM WParam,
				 LPARAM LParam)
{
	LRESULT result = 0;

	switch(Message)
	{
		case WM_CREATE:
		{
			OutputDebugString("WM_CREATE\n");
		} break;

		case WM_SIZE: // when the user changes the size of the window
		{
			OutputDebugString("WM_SIZE\n");

			RECT client_rect;
			GetClientRect(Window, &client_rect);

			int width = client_rect.right - client_rect.left;
			int height = client_rect.bottom - client_rect.top;

			Win32ResizeDIBSection(width, height);

		} break;

		case WM_DESTROY:
		{
			OutputDebugString("WM_DESTROY\n");
			Running = false;
		} break;

		// case WM_SETCURSOR:
		// {
		// 	SetCursor(0);
		// } break;

		case WM_CLOSE: // when user presses the x in the top right hand corner
		{
			OutputDebugString("WM_CLOSE\n");

			/// REFERENCE PostQuitMessage
			/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms644945%28v=vs.85%29.aspx
			// PostQuitMessage(0);

			Running = false;

		} break;

		case WM_ACTIVATEAPP: 
		{
			OutputDebugString("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			OutputDebugString("WM_PAINT\n");

			/// REFERENCE PAINTSTRUCT
			/// http://msdn.microsoft.com/en-us/library/windows/desktop/dd162768(v=vs.85).aspx

			// typedef struct tagPAINTSTRUCT {
			//   HDC  hdc;
			//   BOOL fErase;
			//   RECT rcPaint;
			//   BOOL fRestore;
			//   BOOL fIncUpdate;
			//   BYTE rgbReserved[32];
			// } PAINTSTRUCT, *PPAINTSTRUCT;

			PAINTSTRUCT paint;

			/// REFERENCE BeginPaint
			/// http://msdn.microsoft.com/en-us/library/windows/desktop/dd183362(v=vs.85).aspx

			HDC device_context = BeginPaint(Window, &paint);

			/// REFERENCE PatBlt
			/// http://msdn.microsoft.com/en-us/library/windows/desktop/dd162778%28v=vs.85%29.aspx

			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int width = paint.rcPaint.right - paint.rcPaint.left;
			int height = paint.rcPaint.bottom - paint.rcPaint.top;

			RECT client_rect;
			GetClientRect(Window, &client_rect);

			Win32UpdateWindow(device_context, client_rect, x, y, width, height);

			/// REFERENCE EndPaint
			/// http://msdn.microsoft.com/en-us/library/windows/desktop/dd162598(v=vs.85).aspx

			EndPaint(Window, &paint);


		} break;

		default:
		{
			// OutputDebugString("default\n");
			result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}

	return result;
}


int CALLBACK
WinMain(HINSTANCE Instance,
		HINSTANCE PrevInstance,
		LPSTR CommandLine,
		int ShowCode)
{

	/// REFERENCE WNDCLASS
	/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms633576%28v=vs.85%29.aspx
	WNDCLASS window_class = {}; /// {} initializes struct to 0

	// i'll handle the device context and redraw when window is moved
	window_class.style = CS_HREDRAW|CS_VREDRAW;
	window_class.lpfnWndProc = MainProcCallback;
	window_class.hInstance = Instance; // GetModuleHandle(0) will give the current handle the instance that is running
	// window_class.hIcon =;
	window_class.lpszClassName = "fromScratchWindowClass";

	if(RegisterClass(&window_class))
	{
		/// REFERENCE CreateWindowEx
		/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms632680(v=vs.85).aspx
		HWND window_handle = CreateWindowEx(
			0,
			window_class.lpszClassName,
			"From Scratch",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0);

		if(window_handle)
		{
			/// REFERENCE GetMessage
			/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms644936(v=vs.85).aspx

			/// REFERENCE PeekMessage
			/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms644943%28v=vs.85%29.aspx

			int XOffset = 0;
			int YOffset = 0;
			Running = true;
			while(Running)
			{
				
				MSG message;
				while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if(message.message == WM_QUIT)
					{
						Running = false;
					}

					/// REFERENCE TranslateMessage
					/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms644955(v=vs.85).aspx

					TranslateMessage(&message);

					/// REFERENCE DipatchMessage
					/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms644934(v=vs.85).aspx

					DispatchMessage(&message);
				}

				Win32RenderGradient(XOffset, YOffset);
				HDC device_context = GetDC(window_handle);

				RECT client_rect;
				GetClientRect(window_handle, &client_rect);

				int window_width = client_rect.right - client_rect.left;
				int window_height = client_rect.bottom - client_rect.top;

				Win32UpdateWindow(device_context, client_rect, 0, 0, window_width, window_height);

				ReleaseDC(window_handle, device_context);

				XOffset++;
			}
				
		}
		else
		{
			/// TODO: creating the window failed
		}

	}
	else
	{
		/// TODO: window registration failed
	}

	return (0);
}
