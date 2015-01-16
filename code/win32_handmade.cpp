
#include <windows.h>

#define internal_function static
#define local_persist static
#define global_variable static  // global statics are automatically initialized to 0

global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;


internal_function void
Win32ResizeDIBSection(int Width, int Height)
{

	/// free the global handle before creating a new bitmap

	if(BitmapHandle)
	{
		DeleteObject(BitmapHandle);
	}
	
	if(BitmapDeviceContext != 0)
	{
		/// ask windows for a device context that is compatible with something in this case the screen
		BitmapDeviceContext = CreateCompatibleDC(0);
	}

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	// BitmapInfo.bmiHeader.biPlanes = 0;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	// BitmapInfo.bmiHeader.biSizeImage = 0;
	// BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	// BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	// BitmapInfo.bmiHeader.biClrUsed = 0;
	// BitmapInfo.bmiHeader.biClrImportant = 0;

	HBITMAP bitmap_handle = CreateDIBSection(
		BitmapDeviceContext,
		&BitmapInfo,
		DIB_RGB_COLORS,
		&BitmapMemory,
		0, 0);
}


internal_function void
Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
	/// REFERENCE BitBlt
	/// http://msdn.microsoft.com/en-us/library/windows/desktop/dd183370(v=vs.85).aspx

	/// REFERENCE StretchDIBits
	/// http://msdn.microsoft.com/en-us/library/windows/desktop/dd145121(v=vs.85).aspx

	StretchDIBits(
		DeviceContext,
		X, Y, Width, Height,
		X, Y, Width, Height,
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

			Win32UpdateWindow(device_context, x, y, width, height);

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
	window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
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

			MSG message;
			Running = true;
			while(Running)
			{
				BOOL message_result = GetMessage(&message,
					   0,
					   0,
					   0);
				if(message_result > 0)
				{
					/// REFERENCE TranslateMessage
					/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms644955(v=vs.85).aspx

					TranslateMessage(&message);

					/// REFERENCE DipatchMessage
					/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms644934(v=vs.85).aspx

					DispatchMessage(&message);
				}
				else
				{
					break;
				}
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
