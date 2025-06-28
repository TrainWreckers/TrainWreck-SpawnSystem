modded class SCR_PlayerController
{
	private SCR_BaseGameMode GetGameMode()
	{
		return SCR_BaseGameMode.Cast(GetGame().GetGameMode());
	}
	
	void UpdateFactionSpawnSettings(SpawnSettingsBase settings)
	{	
		// If Server -> We don't have to RPC
		if(Replication.IsServer())
		{
			SCR_BaseGameMode game = GetGameMode();
			game.UpdateFactionSpawnSettings(settings);
		}
		// We'll want to update locally and send update to server
		else
		{
			SCR_BaseGameMode.TW_SpawnSettings = settings;
			Rpc(Rpc_Server_OnFactionSettingsChanged, TW_Util.ToJson(settings, true));
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_Server_OnFactionSettingsChanged(string settings)
	{
		GetGameMode().UpdateFactionSpawnSettings(SpawnSettingsBase.LoadFromFile(settings));
	}
};