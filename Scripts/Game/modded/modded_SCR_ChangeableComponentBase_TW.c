class TW_ChangeableComponentUpdateData
{
	/*!
		Since we don't have a generic object in enfusion
		this works...
	*/
	string GetValueAsString();
};

class TW_ChangeableComponentUpdateData_Generic<Class T> : TW_ChangeableComponentUpdateData
{
	T Value;
	
	void TW_ChangeableComponentUpdateData_Generic(T value)
	{
		this.Value = value;
	}
	
	override string GetValueAsString()
	{
		return string.Format("%1", Value);
	}
};

class TW_ChangeableComponentUpdateEvent
{
	SCR_ChangeableComponentBase Component;
	TW_ChangeableComponentUpdateData Data;
	
	void TW_ChangeableComponentUpdateEvent(SCR_ChangeableComponentBase base, TW_ChangeableComponentUpdateData data)
	{
		this.Component = base;
		this.Data = data;
	}
};

modded class SCR_ChangeableComponentBase
{
	string ComponentKey;
	
	/*!
		Way to generically pass values around
	*/
	ref ScriptInvoker<TW_ChangeableComponentUpdateEvent> m_TW_OnChanged = new ScriptInvoker<TW_ChangeableComponentUpdateEvent>();	
};

modded class SCR_ToolboxComponent
{
	override void OnElementChanged(SCR_ButtonBaseComponent comp, bool state)
	{
		// Do not invoke on false state when not having a multiselection
		if (!m_bAllowMultiselection && !state)
			return;
		
		int i = m_aSelectionElements.Find(comp);
		if (m_bAllowMultiselection)
			m_OnChanged.Invoke(this, i, state);
		else
			m_OnChanged.Invoke(this, i);
		
		ref TW_ChangeableComponentUpdateData_Generic<bool> data = new TW_ChangeableComponentUpdateData_Generic<bool>(state);
		m_TW_OnChanged.Invoke(new TW_ChangeableComponentUpdateEvent(this, data));
	}
};

modded class SCR_CheckboxComponent
{
	override void OnElementChanged(SCR_ButtonBaseComponent comp, bool state)
	{
		if (!state)
			return;
		
		bool selected = m_aSelectionElements.Find(comp);
		m_OnChanged.Invoke(this, selected);
		
		ref TW_ChangeableComponentUpdateData_Generic<bool> data = new TW_ChangeableComponentUpdateData_Generic<bool>(state);
		m_TW_OnChanged.Invoke(new TW_ChangeableComponentUpdateEvent(this, data));
	}
};

modded class SCR_ComboBoxComponent
{
	private override void OnElementSelected(SCR_ButtonTextComponent comp)
	{
		int i = m_aElementWidgets.Find(comp.m_wRoot);
		if (i > -1)
		{
			SetCurrentItem(i, false, true);
			if (m_wText)
				m_wText.SetText(m_aElementNames[i]);
		}

		CloseList();
		m_OnChanged.Invoke(this, i);
		
		ref TW_ChangeableComponentUpdateData_Generic<int> data = new TW_ChangeableComponentUpdateData_Generic<int>(i);
		m_TW_OnChanged.Invoke(new TW_ChangeableComponentUpdateEvent(this, data));
	}
}

modded class SCR_EditBoxComponent
{
	override void OnValueChanged()
	{
		if (m_Hint)
			m_Hint.SetVisible(false);
		
		string text = GetEditBoxText();
		m_OnChanged.Invoke(this, text);

		if (m_bValidInput)
			return;
		
		AnimateWidget.Opacity(m_wWarningIcon, 0, m_fColorsAnimationTime, true);
		
		AnimateWidget.Color(m_wColorOverlay, COLOR_VALID_INPUT, m_fColorsAnimationTime);
		m_bValidInput = true;
		
		ref TW_ChangeableComponentUpdateData_Generic<string> data = new TW_ChangeableComponentUpdateData_Generic<string>(text);
		m_TW_OnChanged.Invoke(new TW_ChangeableComponentUpdateEvent(this, data));
	}
}

modded class SCR_SliderComponent
{
	protected override void OnValueChanged(Widget w)
	{
		float value = UpdateValue();

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Can lead to severe sound spamming based on speed of user interaction
		
		if (m_sChangeSound != string.Empty && !float.AlmostEqual(m_fOldValue, value))
			PlaySound(m_sChangeSound);
		
//---- REFACTOR NOTE END ----
		
		m_fOldValue = value;
		
		m_OnChanged.Invoke(this, value);
		
		ref TW_ChangeableComponentUpdateData_Generic<float> data = new TW_ChangeableComponentUpdateData_Generic<float>(value);
		m_TW_OnChanged.Invoke(new TW_ChangeableComponentUpdateEvent(this, data));
	}
	
	protected override void OnValueFinal(Widget w)
	{
		float value;
		if (m_wSlider)
			value = m_wSlider.GetCurrent();

		if (m_OnChangedFinal)
			m_OnChangedFinal.Invoke(this, value);
		
		ref TW_ChangeableComponentUpdateData_Generic<float> data = new TW_ChangeableComponentUpdateData_Generic<float>(value);
		m_TW_OnChanged.Invoke(new TW_ChangeableComponentUpdateEvent(this, data));
	}
};

modded class SCR_SpinBoxComponent
{
	protected override bool SetCurrentItem_Internal(int i, bool playSound, bool animate, bool invokeOnChanged)
	{
		int lastIndex = m_iSelectedItem;
		if (!super.SetCurrentItem(i, playSound, animate))
			return false;

		if (m_wText)
			m_wText.SetText(m_aElementNames[i]);

		if (m_bShowHints)
			UpdateHintBar(i, lastIndex);

		EnableArrows(i, animate);
		
		if (invokeOnChanged)
			m_OnChanged.Invoke(this, m_iSelectedItem);
		
		ref TW_ChangeableComponentUpdateData_Generic<int> data = new TW_ChangeableComponentUpdateData_Generic<int>(m_iSelectedItem);
		m_TW_OnChanged.Invoke(new TW_ChangeableComponentUpdateEvent(this, data));
		
		return true;
	}
};