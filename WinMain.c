#include <stdio.h>
#include <time.h>

#include <Windows.h>
#include <wingdi.h>
#include <WinUser.h>

#define LIFE_SIZE		20
#define LIFE_WIDTH		50
#define LIFE_HEIGHT		50
#define LIFE_DELAY		100
#define BUFFER_SIZE		200

#define ID(x,y) ((y)*(LIFE_WIDTH)+(x))

#define NOLIFE 0
#define LIFE 1

typedef unsigned char life_t;
typedef unsigned long tick_t;

LPCSTR szMyWindowApplicationName = "LifeTheGame";

HINSTANCE	hMainInstance;
HWND		hMainWindow, hConsole;
HBITMAP		hBmpCanvas;

BOOL bPaused = FALSE;

tick_t ticks = (tick_t)0ul;

life_t life[LIFE_WIDTH][LIFE_HEIGHT];

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndPaint(HWND hWnd, HDC hDC, PAINTSTRUCT ps, RECT rcClient)
{
	Rectangle(hDC, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

	HPEN hPen = CreatePen(PS_SOLID,1,RGB(100,100,100));
	SelectObject(hDC,hPen);

	for(int i = 1;i < LIFE_HEIGHT;i++)
	{
		int y = rcClient.top + i * LIFE_SIZE;
		MoveToEx(hDC, rcClient.left,y, NULL);
		LineTo(hDC, rcClient.right,y);
	}

	for(int i = 1;i < LIFE_WIDTH; i++)
	{
		int x = rcClient.left + i * LIFE_SIZE;
		MoveToEx(hDC, x,rcClient.top, NULL);
		LineTo(hDC,x, rcClient.bottom);
	}

	SIZE sizeText;
	char buffer[BUFFER_SIZE];

	SetTextColor(hDC, RGB(0, 0, 0));
	SetBkMode(hDC, TRANSPARENT);
	SelectObject(hDC,GetStockObject(DEFAULT_GUI_FONT));

	HBRUSH hBrush = CreateSolidBrush(RGB(0,0,0));
	SelectObject(hDC,hBrush);

	for(int x = 0;x < LIFE_WIDTH;x++)
		for(int y = 0;y < LIFE_HEIGHT;y++)
			if(life[x][y] == LIFE)
				Rectangle(hDC,rcClient.left + x * LIFE_SIZE,rcClient.top + y * LIFE_SIZE,rcClient.left + (x + 1) * LIFE_SIZE,rcClient.top + (y+1) * LIFE_SIZE);

	SetTextColor(hDC, RGB(0, 0, 0));

	snprintf(buffer,BUFFER_SIZE, "DELAY: %d ms", LIFE_DELAY);
	GetTextExtentPoint32A(hDC, buffer, strlen(buffer), &sizeText);
	TextOutA(hDC, rcClient.left + 5, rcClient.top + 5, buffer, strlen(buffer));
	
	snprintf(buffer,BUFFER_SIZE, "TICKS: %d", ticks);
	GetTextExtentPoint32A(hDC, buffer, strlen(buffer), &sizeText);
	TextOutA(hDC, rcClient.left + 5, rcClient.top + 25, buffer, strlen(buffer));
	
	if(bPaused)
	{
		snprintf(buffer,BUFFER_SIZE, "PAUSED");
		GetTextExtentPoint32A(hDC, buffer, strlen(buffer), &sizeText);
		TextOutA(hDC, rcClient.left + 5, rcClient.top + 45, buffer, strlen(buffer));
	}

	DeleteObject(hPen);
	DeleteObject(hBrush);

	return 0;
}

LRESULT WndThink(HWND hWnd)
{
	life_t new[LIFE_WIDTH][LIFE_HEIGHT];

	for(int x = 0;x < LIFE_WIDTH;x++)
		for(int y = 0;y < LIFE_HEIGHT;y++)
		{
			int neigh = 0;
			for (int i = -1; i <= 1; i++)
				for (int j = -1; j <= 1; j++)
				{
					if (i == 0 && j == 0)
						continue;

					if (x + i < 0 || x + i >= LIFE_WIDTH || y + j < 0 || y + j >= LIFE_HEIGHT)
						continue;

					if (life[x + i][y + j])
						neigh++;
				}


			switch (life[x][y])
			{
			case LIFE:
				new[x][y] = (neigh < 2 || neigh > 3) ? NOLIFE : LIFE;

				break;
			case NOLIFE:
				new[x][y] = (neigh == 3) ? LIFE : NOLIFE;
					
				break;

			default:
				break;
			}
		}

	for(int x = 0;x < LIFE_WIDTH;x++)
		for(int y = 0;y < LIFE_HEIGHT;y++)
			life[x][y] = new[x][y];

	ticks++;
			
	InvalidateRect(hWnd,NULL,FALSE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPCSTR lpCmdLine, int nCmdShow)
{
	hMainInstance = hInstance;
	hConsole = GetConsoleWindow();

	WNDCLASSEXA wcex;
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIconA(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursorA(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "LifeTheGameApp";
	wcex.hIconSm = LoadIconA(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassExA(&wcex))
	{
		MessageBoxA(NULL,
			"Registration window class failed!",
			szMyWindowApplicationName,
			MB_ICONSTOP | MB_OK);
		
		return EXIT_FAILURE;
	}

	RECT windowSize;
	windowSize.top = windowSize.left = 0;
	windowSize.right = LIFE_WIDTH * LIFE_SIZE;
	windowSize.bottom = LIFE_HEIGHT * LIFE_SIZE;

	AdjustWindowRectEx(&windowSize,WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,FALSE,WS_EX_OVERLAPPEDWINDOW);
	
	hMainWindow = CreateWindowExA(
		WS_EX_OVERLAPPEDWINDOW,
		wcex.lpszClassName,
		szMyWindowApplicationName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowSize.right-windowSize.left, windowSize.bottom-windowSize.top,
		NULL,
		NULL,
		wcex.hInstance,
		NULL
	);

	if (hMainWindow == NULL)
	{
		MessageBoxA(NULL,
			TEXT("Creating window failed!"),
			szMyWindowApplicationName,
			MB_ICONSTOP | MB_OK);

		return EXIT_FAILURE;
	}

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	MSG msg;

	while (GetMessageA(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
	
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtrA(hWnd, GWLP_HINSTANCE);

	switch (message)
	{
		case WM_CREATE:
		{
			SetTimer(hWnd,1,LIFE_DELAY,NULL);

			life[3][2] = LIFE;
			life[3][3] = LIFE;
			life[3][4] = LIFE;

			return 0;
		}

		case WM_COMMAND:
		{
			return 0;
		}

		case WM_PAINT:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);

			PAINTSTRUCT ps;

			HDC hDC = BeginPaint(hWnd, &ps);
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP bmp = CreateCompatibleBitmap(hDC, rect.right - rect.left, rect.bottom - rect.top);
			SelectObject(memDC, bmp);

			LRESULT result = WndPaint(hWnd, memDC, ps, rect);

			BitBlt(hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, memDC, 0, 0, SRCCOPY);

			DeleteObject(bmp);
			DeleteDC(memDC);

			EndPaint(hWnd, &ps);

			return result;
		}

		case WM_TIMER:
		{
			return bPaused ? 0 : WndThink(hWnd);
		}

		case WM_MBUTTONDOWN:
		{
			bPaused = !bPaused;

			InvalidateRect(hWnd,NULL,FALSE);

			return 0;
		}

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MOUSEMOVE:
		{
			if(wParam & MK_LBUTTON || wParam & MK_RBUTTON)
			{
				POINTS points = MAKEPOINTS(lParam);
				
				int x = min(points.x / LIFE_SIZE,LIFE_WIDTH);
				int y = min(points.y / LIFE_SIZE,LIFE_HEIGHT);

				if(wParam & MK_LBUTTON)
					life[x][y] = LIFE;
				else if(wParam & MK_RBUTTON)
					life[x][y] = NOLIFE;

				InvalidateRect(hWnd,NULL,FALSE);
			}

			return 0;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(EXIT_SUCCESS);

			return 0;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}