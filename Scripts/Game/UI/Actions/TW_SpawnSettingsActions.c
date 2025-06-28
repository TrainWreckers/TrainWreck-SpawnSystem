class TW_SpawnSettings_ScriptedUserAction : ScriptedUserAction
{
	[Attribute(defvalue: "{11DFF8493E5F6F83}UI/SpawnSettingsLayout.layout")]
	protected ResourceName m_Layout;
	protected ref Widget m_Display;
};

class TW_SpawnSettings_DisplayAction : TW_SpawnSettings_ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if(m_Display)
		{
			delete m_Display;
			return;
		}
		
		m_Display = GetGame().GetWorkspace().CreateWidgets(m_Layout);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if(m_Display)
			outName = "Close TrainWreck Spawn Settings";
		else
			outName = "Show TrainWreck Spawn Settings";
		return true;
	};		
};

class TW_SpawnSettings_MenuAction : TW_SpawnSettings_ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if(m_Display)
		{
			delete m_Display;
			return;
		}
		
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.TW_SpawnSettingsMenuUI);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if(m_Display)
			outName = "Hide TrainWreck Spawn Settings";
		else
			outName = "Show TrainWreck Spawn Settings";
		return true;
	}
};

class TW_SpawnSettings_DialogAction : TW_SpawnSettings_ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if(m_Display)
		{
			delete m_Display;
			return;
		}
		
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.TW_SpawnSettingsMenuUI, DialogPriority.INFORMATIVE, 0, true);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if(m_Display)
			outName = "Hide TrainWreck Spawn Settings";
		else
			outName = "Show TrainWreck Spawn Settings";
		return true;
	}
};

