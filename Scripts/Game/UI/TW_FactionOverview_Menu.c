class TW_FactionOverview_Menu: MenuBase
{
	private SCR_InputButtonComponent saveButton;
	
	protected override void OnMenuOpen()
	{
		Widget rootWidget = GetRootWidget();
		
		if(!rootWidget) return;
		Widget widget = GetGame().GetWorkspace().CreateWidgets("{76F362470F718CE5}UI/layouts/FactionSettings/FactionOverview.layout", rootWidget);
		
		saveButton = SCR_InputButtonComponent.Cast(rootWidget.FindAnyWidget("SaveButton").FindHandler(SCR_InputButtonComponent));
		AddListeners();
	}
	
	private void AddListeners()
	{
		InputManager manager = GetGame().GetInputManager();
		if(!manager) return;
		manager.AddActionListener("MenuExit", EActionTrigger.PRESSED, OnMenuClose);
		saveButton.m_OnClicked.Insert(OnMenuClose);
		
	}
	
	private void RemoveListeners()
	{
		InputManager manager = GetGame().GetInputManager();
		if(!manager) return;
		manager.RemoveActionListener("MenuExit", EActionTrigger.PRESSED, OnMenuClose);
		saveButton.m_OnClicked.Remove(OnMenuClose);
	}
	
	protected override void OnMenuClose()
	{
		ref TW_SpawnSettingsInterface interface = SpawnSettingsManager.GetInstance().GetInterface();
		interface.SaveSettings();
		Close();
	}
};