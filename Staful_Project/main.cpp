//�C���N���[�h
#include <Windows.h>
#include <string>
#include <stdlib.h>
#include "Engine/Direct3D.h"
#include "Engine/Input.h"
#include "Engine/Camera.h"
#include "Engine/RootJob.h"
#include "Engine/Model.h"
#include "Engine/SceneManager.h"
#include "DirectXCollision.h"
#include "resource.h"
#include "MapEditScene.h"
#include "Stage.h"
#include "SceneTransition.h"

//�����J
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")

//�萔�錾
const char* WIN_CLASS_NAME = "SampleGame";  //�E�B���h�E�N���X��
const char* GAME_TITLE = "�T���v���Q�[��";
const int WINDOW_WIDTH = 800;  //�E�B���h�E�̕�
const int WINDOW_HEIGHT = 600; //�E�B���h�E�̍���

RootJob* pRootJob = nullptr;

HINSTANCE ghInstance;

//�v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

ID3D11Device*           pDevice_;		//�f�o�C�X
ID3D11DeviceContext*    pContext_;		//�f�o�C�X�R���e�L�X�g
IDXGISwapChain*         pSwapChain_;		//�X���b�v�`�F�C��
ID3D11RenderTargetView* pRenderTargetView_;	//�����_�[�^�[�Q�b�g�r���[

//�G���g���[�|�C���g
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    ghInstance = hInstance;
    //XMVECTOR beginP = XMVectorSet(10, 5, 1, 0);
    //XMVECTOR dir = XMVectorSet(0, -1, 0, 0);
    //XMVECTOR p0 = XMVectorSet(0, 0, 0, 0);
    //XMVECTOR p1 = XMVectorSet(0, 0, 3, 0);
    //XMVECTOR p2 = XMVectorSet(3, 0, 0, 0);
    //float dist = 0;

    //bool result = TriangleTests::Intersects(beginP, dir, p0, p1, p2, dist);

    //int a = 0;

    //�E�B���h�E�N���X�i�݌v�}�j���쐬
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);             //���̍\���̂̃T�C�Y
    wc.hInstance = hInstance;                   //�C���X�^���X�n���h��
    wc.lpszClassName = WIN_CLASS_NAME;            //�E�B���h�E�N���X��
    wc.lpfnWndProc = WndProc;                   //�E�B���h�E�v���V�[�W��
    wc.style = CS_VREDRAW | CS_HREDRAW;         //�X�^�C���i�f�t�H���g�j
    wc.hIcon = LoadIcon(NULL, IDI_QUESTION);        //�A�C�R��
    wc.hIconSm = LoadIcon(NULL, IDI_QUESTION);   //�������A�C�R��
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);   //�}�E�X�J�[�\��
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); //���j���[�i�Ȃ��j
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //�w�i�i���j

    RegisterClassEx(&wc);  //�N���X��o�^

    //�E�B���h�E�T�C�Y�̌v�Z
    RECT winRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&winRect, WS_OVERLAPPEDWINDOW, TRUE);
    int winW = winRect.right - winRect.left;     //�E�B���h�E��
    int winH = winRect.bottom - winRect.top;     //�E�B���h�E����

  //�E�B���h�E���쐬
    HWND hWnd = CreateWindow(
        WIN_CLASS_NAME,         //�E�B���h�E�N���X��
        GAME_TITLE,     //�^�C�g���o�[�ɕ\��������e
        WS_OVERLAPPEDWINDOW, //�X�^�C���i���ʂ̃E�B���h�E�j
        CW_USEDEFAULT,       //�\���ʒu���i���܂����j
        CW_USEDEFAULT,       //�\���ʒu��i���܂����j
        winW,                 //�E�B���h�E��
        winH,                 //�E�B���h�E����
        NULL,                //�e�E�C���h�E�i�Ȃ��j
        NULL,                //���j���[�i�Ȃ��j
        hInstance,           //�C���X�^���X
        NULL                 //�p�����[�^�i�Ȃ��j
    );

  //�E�B���h�E��\��
    ShowWindow(hWnd, nCmdShow);

    HRESULT hr;

    //Direct3D������
    hr = Direct3D::Initialize(WINDOW_WIDTH, WINDOW_HEIGHT, hWnd);
    if (FAILED(hr))
    {
        //���s�����Ƃ��̏���
        PostQuitMessage(0);
    }
    Input::Initialize(hWnd);

    pRootJob = new RootJob(nullptr);
    pRootJob->Initialize();

    //�J�����A�N��
    Camera::Initialize();

    //Camera::SetPosition(XMFLOAT3(0, 0, -10));
    
  //���b�Z�[�W���[�v�i�����N����̂�҂j
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        //���b�Z�[�W����
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        //���b�Z�[�W�Ȃ�
        else
        {
            static bool is_be_daialog = false;
            if (((SceneManager*)pRootJob->FindChildObject("SceneManager"))->GetCurrentSceneID() == SCENE_ID_TRANSITION)
            {
                if (((SceneTransition*)pRootJob->FindChildObject("SceneTransition"))->GetSceneState() == SCENESTATE::SCENE_MAPEDIT1_DELAY &&
                    !is_be_daialog)
                {
                    HWND hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)DialogProc);
                    is_be_daialog = true;
                }
                else if(((SceneTransition*)pRootJob->FindChildObject("SceneTransition"))->GetSceneState() == SCENESTATE::SCENE_STAGE1)
                {
                    is_be_daialog = false;
                }
            }

            timeBeginPeriod(1);

            static DWORD countFps = 0;

            static DWORD startTime = timeGetTime();
            DWORD nowTime = timeGetTime();
            static DWORD lastUpdateTime = nowTime;

            if (nowTime - startTime >= 1000)
            {
                char str[16];
                wsprintf(str, "%u", countFps);

                SetWindowText(hWnd, str);

                countFps = 0;
                startTime = nowTime;
            }

            if ((nowTime - lastUpdateTime) * 60 <= 1000)
            {
                continue;
            }
            lastUpdateTime = nowTime;

            countFps++;

            timeEndPeriod(1);

            //�J�����A�X�V
            Camera::Update();

            //���́A�X�V
            Input::Update();

            pRootJob->UpdateSub();

            //�Q�[���̏���
            Direct3D::BeginDraw();

            //���[�g�W���u���炷�ׂẴI�u�W�F�N�g�̃h���[���Ă�
            pRootJob->DrawSub();

            //�`�揈��
            Direct3D::EndDraw();
        }
    }

    //�������
    Model::Release();
    pRootJob->ReleaseSub();
    Input::Release();
    Direct3D::Release();

	return 0;
}


//�E�B���h�E�v���V�[�W���i�������������ɂ�΂��֐��j
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_MOUSEMOVE:
        Input::SetMousePosition(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_DESTROY:
        PostQuitMessage(0);  //�v���O�����I��
        break;
        
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

BOOL CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    return ((MapEditScene*)pRootJob->FindObject("MapEditScene"))->DialogProc(hDlg, msg, wp, lp);
}