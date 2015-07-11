#include <Base/Base.h>
#pragma hdrstop
#include <typeinfo.h>	// typeid()
#include <Driver/Driver.h>
#include <GameUtil/GameUtil.h>

ERet MainMode::Initialize()
{
	return ALL_OK;
}
void MainMode::Shutdown()
{
}

void DevConsoleState::Update( double deltaSeconds )
{
}
void DevConsoleState::RenderGUI()
{
	static bool show_test_window = true;
	static bool show_another_window = false;

	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	{
		static float f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		show_test_window ^= ImGui::Button("Test Window");
		show_another_window ^= ImGui::Button("Another Window");

		// Calculate and show frame rate
		static float ms_per_frame[120] = { 0 };
		static int ms_per_frame_idx = 0;
		static float ms_per_frame_accum = 0.0f;
		ms_per_frame_accum -= ms_per_frame[ms_per_frame_idx];
		ms_per_frame[ms_per_frame_idx] = ImGui::GetIO().DeltaTime * 1000.0f;
		ms_per_frame_accum += ms_per_frame[ms_per_frame_idx];
		ms_per_frame_idx = (ms_per_frame_idx + 1) % 120;
		const float ms_per_frame_avg = ms_per_frame_accum / 120;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ms_per_frame_avg, 1000.0f / ms_per_frame_avg);
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window, ImVec2(200,100));
		ImGui::Text("Hello");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20));
		ImGui::ShowTestWindow(&show_test_window);
	}
}
#if 0

ERet EscapeMenu::Initialize( Clump* shaders )
{
	mxDO(m_gui.Initialize(shaders));
	return ALL_OK;
}

void EscapeMenu::Shutdown()
{
	m_gui.Shutdown();
}

void EscapeMenu::Update( double deltaSeconds )
{
	const bool bHasFocus = WindowsDriver::HasFocus();

	// avoid assertion: IM_ASSERT(GImGui.CurrentWindow != NULL);
	//if( WindowsDriver::HasFocus() )
	{
		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ms_per_frame_avg, 1000.0f / ms_per_frame_avg);
		//ImGui::Text("Hello, world!");
		//static float f;
		//ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
	}

	if(bHasFocus)
	{
		m_gui.Update();



		static bool show_test_window = true;
		static bool show_another_window = false;

		// 1. Show a simple window
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		{
			static float f;
			ImGui::Text("Hello, world!");
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			show_test_window ^= ImGui::Button("Test Window");
			show_another_window ^= ImGui::Button("Another Window");

			// Calculate and show frame rate
			static float ms_per_frame[120] = { 0 };
			static int ms_per_frame_idx = 0;
			static float ms_per_frame_accum = 0.0f;
			ms_per_frame_accum -= ms_per_frame[ms_per_frame_idx];
			ms_per_frame[ms_per_frame_idx] = ImGui::GetIO().DeltaTime * 1000.0f;
			ms_per_frame_accum += ms_per_frame[ms_per_frame_idx];
			ms_per_frame_idx = (ms_per_frame_idx + 1) % 120;
			const float ms_per_frame_avg = ms_per_frame_accum / 120;
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ms_per_frame_avg, 1000.0f / ms_per_frame_avg);
		}

		// 2. Show another simple window, this time using an explicit Begin/End pair
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window, ImVec2(200,100));
			ImGui::Text("Hello");
			ImGui::End();
		}

		// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
		if (show_test_window)
		{
			ImGui::SetNextWindowPos(ImVec2(650, 20));
			ImGui::ShowTestWindow(&show_test_window);
		}
	}
}

void EscapeMenu::Render()
{
	m_gui.Render();
}
#endif


ERet StateManager::Initialize()
{
	return ALL_OK;
}
void StateManager::Shutdown()
{
	//mxASSERT(m_stateStack.IsEmpty());
}
AClientState* StateManager::GetCurrentState()
{
	AClientState* state = m_stateStack.GetLast();
	return state;
}
void StateManager::PushState( AClientState* state )
{
	m_stateStack.Add( state );
	DBGOUT("PushState(%s): mask=%d", typeid(*state).name(), GetCurrentState()->GetStateMask());
}
AClientState* StateManager::PopState()
{
	AClientState* state = m_stateStack.GetLast();
	m_stateStack.Pop();
	DBGOUT("PushState(%s): mask=%d",  typeid(*state).name(), GetCurrentState()->GetStateMask());
	return state;
}
