// Api_Hook.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Api_Hook.h"

#define MAX_LOADSTRING 100

// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

char *g_szHello = "Hello world!";
char *g_szTitle = "Page404";

char *g_szClassName = "MSPaintApp";

char *g_szKernel32 = "Kernel32";
char *g_szUser32 = "User32";

char *g_szSleep = "Sleep";
char *g_szMsgBox = "MessageBoxW";

char *g_hInst;

typedef struct tagTrueAddr
{

	BYTE g_JmpCode;
	long g_JmpOffset;

}TrueAddr, *PTrueAddr;

__declspec(naked) void __stdcall InjectCode(FARPROC lpMsgBox){

	__asm{

		//������Ĵ�����������
		pushad

		//----  ��̬�ض�λ(����ʱ(call NEXT)�ĵ�ַ ��ȥ ����ʱ(offset)�ĵ�ַ), �����ƫ��
		//INJECTCODE_BEGIN �� INJECTCODE_END ��δ�����ע�뵽ɨ�׵��ڴ浱��ȥ��, ����ƫ�Ƶ�ַ�������Լ���hello.exe�϶���һ��, ����Ҫ�ö�̬�ض�λ�ķ�ʽ������ƫ����
		//�����������ϵͳ�� LoadLibrary->GetProcAddress �õ� MessageBoxA �� Sleep ��ϵͳ api �Ĺ̶���ַ
		//���, �� ���ƫ�Ƶ�ַ + �̶���ַ, �������ǳ�������ʱ, ���õ�ϵͳapi��ַ.
		//��������, ����ע���κε�exe��, ���Ǽ���Ҫע��exe����ʱ�����õ�ϵͳapi��ַ.
		call NEXT
		NEXT :
		pop ebx
		sub ebx, offset NEXT

		push MB_OK
		push NULL
		mov eax, [esp + 2ch]  //esp + 2ch: ����ͼƬ��ȫ·��,��Ϊhook����CreateFileW���ϵͳapi,����,·�����ڸú�������
		push eax
		push NULL
		call[InjectCode - 4 + ebx]  //InjectCode - 4 :Ϊ����д���ڴ�ʱ,MessageBox��ϵͳUser32.dll�е�λ��

		//��ԭ���Ĵ�����������
		popad
		//�� hook �滻������һ�д���,����Ҫд��ȥ.
		mov   eax, 10362A5h

		//------101d1a2h hookĿ���ڴ��е���һ�е�ַ.
        //push + ret �൱�� jmp
        //���ֱ���� jmp ,��ô��Ҫ�����������ƫ�Ƶ�ַ.
        //������� jmp ,��ô,���Ȱ���ת��ַ���ȴ�ŵ��Ĵ���,�ٵ���,��: mov ecx,101d1a2h  jmp ecx ,����,����ȷ�� ecx �ں����Ƿ����õ�,����õ���,ֵ�ᱻ����.
		push  101d1a2h
		ret
	}

}

void Inject(){

	FARPROC g_lpMsgBox;
	FARPROC g_lpSleep;

	//���ҵ��Ĵ��ھ��
	HWND hWnd;
	//���̵ı�ʶ��
	DWORD dwPID;
	//�򿪽��̵ľ��
	HANDLE hProcess;
	//�������ڴ�Ļ���ַ
	LPVOID lpMem;
	HANDLE hThread;
	DWORD nInjectCodeSize;
	DWORD dwOld;
	BOOL bRet;
	HMODULE hMoudle;

	try{

		//���� FindWindow api����
		hWnd = FindWindow(g_szClassName, NULL);
		if (hWnd == NULL){
			throw;
		}
		//�õ�Ŀ����̵�ID
		GetWindowThreadProcessId(hWnd, &dwPID);
		//���� OpenProcess api����,������������֮�����ϵ
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
		if (hProcess == NULL){
			throw;
		}

		//����Ҫע�뵽Ŀ���ڴ��еĴ���ĳ���
		nInjectCodeSize = ((long)Inject - (long)InjectCode);

		//���� VirtualAllocEx api����, �����ڴ�ռ�
	    //����� @hProcess Ϊ���ҵ��Ĵ���(����ͼ)�Ľ��̾��, �������ڻ�ͼ���������ڴ�ռ�
		lpMem = VirtualAllocEx(hProcess, NULL, nInjectCodeSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (lpMem == NULL){
			throw;
		}

		hMoudle = LoadLibrary(g_szUser32);
		if (hMoudle == NULL){
			throw;
		}
		//�õ� User32 �е� MessageBox ������ ��ַ
		g_lpMsgBox = GetProcAddress(hMoudle, g_szMsgBox);
		if (g_lpMsgBox == NULL){
			throw;
		}

		//���� WriteProcessMemory api����,д����뵽��ͼ���ڴ�
		bRet = WriteProcessMemory(hProcess, (LPVOID)((int)lpMem + 4), (LPCVOID)InjectCode, nInjectCodeSize+4, NULL);
		if (bRet == FALSE){
			throw;
		}

		//�� MessageBox������ д���ڴ�
		bRet = WriteProcessMemory(hProcess, lpMem, (LPCVOID)&g_lpMsgBox, sizeof(FARPROC), NULL);
		if (bRet == FALSE){
			throw;
		}

		//---�����Լ�ƴ�յ�5���ֽڵ�ָ��
		TrueAddr trueAddr;
		trueAddr.g_JmpCode = 0xe9; //�� jmp �Ļ������� e9,��ռ 5���ֽ� , �� jmp �Ļ������� EB,��ռ 2���ֽ�
		long writeFileW_NextLine = 0x101d19d + 0x5; //101d19dh ��ַΪ writeFileW �ĺ���ָ�������ڴ���ַ,�� jmp �Ļ������� e9,��ռ 5���ֽ�.
		long runCodePosition = (long)lpMem + 0x4;
		//g_JmpOffset Ϊ�滻��� jmp ����� 4���ֽڵ�ֵַ (jmp ����ĵ�ַ�����ƫ�Ƶ�ַ)
		trueAddr.g_JmpOffset = runCodePosition - writeFileW_NextLine;

		//101d19dh ��ַΪ writeFileW �ĺ���ָ�������ڴ���ַ.
		LPVOID nAddr = (LPVOID)0x101d19d;

		//д���ڴ�,�滻 0x101d19d �ڴ洦�� 5 ��ָ��
		bRet = WriteProcessMemory(hProcess, nAddr , (LPCVOID)(&trueAddr.g_JmpCode), 5, NULL);
		if (bRet == FALSE){
			throw;
		}

	}
	catch (...){

		if (hThread == NULL){
			CloseHandle(hThread);
			hThread = NULL;
		}

		//if (lpMem == NULL){
		//	VirtualFreeEx(hProcess, lpMem, nInjectCodeSize, MEM_RELEASE);
		//	lpMem = NULL;
		//}

		if (hProcess == NULL){
			CloseHandle(hProcess);
			hProcess = NULL;
		}

	}

	return;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HFONT hFont;

	switch (message)
	{
	case WM_LBUTTONDOWN:
		Inject();

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �����˵�ѡ��: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  �ڴ���������ͼ����...

		LOGFONT logfont; //�ı��������
		ZeroMemory(&logfont, sizeof(LOGFONT));
		logfont.lfCharSet = GB2312_CHARSET;
		logfont.lfHeight = -16; //��������Ĵ�С
		hFont = CreateFontIndirect(&logfont);
		::SetTextColor(hdc, RGB(255, 0, 0));
		::SetBkColor(hdc, RGB(200, 200, 0));
		::SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, hFont);

		RECT  rt;
		GetClientRect(hWnd, &rt);
		DrawText(hdc, TEXT("�ȴ򿪻�ͼ���,���������Լ�����Ľ����ϵ���������,���������ͼƬ���Ϊ,�ᵯ��������ͼƬ��ȫ·��."), -1, &rt, DT_CENTER);

		DeleteObject(hFont);

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_API_HOOK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_API_HOOK));

	// ����Ϣѭ��: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_API_HOOK));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_API_HOOK);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����:  InitInstance(HINSTANCE, int)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//


// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
