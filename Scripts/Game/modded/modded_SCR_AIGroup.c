modded class SCR_AIGroup
{
	protected bool m_ExcludeFromWanderingSystem;
	protected bool m_ExcludeFromGlobalCount;
	protected bool m_IsWanderer = false;
	protected bool m_IsManagedBySpawnSystem;
	
	bool IsWanderer() { return m_IsWanderer; }
	void SetWanderer(bool value) { m_IsWanderer = value; }
	
	//! Is this group currently managed by TW Spawn System(s)
	bool IsManagedBySpawnSystem() { return m_IsManagedBySpawnSystem; }
	
	void SetIgnoreGlobalCount(bool value)
	{
		m_ExcludeFromGlobalCount = value;
	}
	
	void SetIgnoreWandering(bool value)
	{
		m_ExcludeFromWanderingSystem = value;
	}
	
	void SetIsManagedBySpawnSystem(bool value)
	{
		m_IsManagedBySpawnSystem = value;
	}
	
	bool IgnoreGlobalCount() { return m_ExcludeFromGlobalCount; }
	bool IgnoreWanderingSystem() { return m_ExcludeFromWanderingSystem; }
};