class TW_BaseMenu_Handler
{
	void ConsumeValues(typename type, notnull map<string, string> values);
}

class TW_BaseMenu : MenuBase
{
	protected static const string CONTENT_AREA = "ContentArea";
	
	protected static const string WIDGET_STRING_PREFAB = "";
	protected static const string WIDGET_INTEGER_PREFAB = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	protected static const string WIDGET_FLOAT_PREFAB = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	protected static const string WIDGET_ENUM_PREFAB = "";
	protected static const string WIDGET_BOOL_PREFAB = "{5D5055E10FD00549}UI/layouts/WidgetLibrary/ToolBoxes/WLib_Checkbox.layout";
	protected static const string WIDGET_FLAGS_PREFAB = "";
	
	protected Widget _contentAreaWidget;
	protected Widget _rootWidget;
	
	protected string GetSchemaPath();
	
	/*!
		Variable Name --> Value
		Can use a mapper function that consumes values from this mapper
	*/
	protected ref map<string, string> _valueMap = new map<string, string>();
		
	private void OnComponentValueChanged(TW_ChangeableComponentUpdateEvent e)
	{
		if(!e) return;
		
		if(!_valueMap.Contains(e.Component.ComponentKey))
			return;
		
		
	}
	
	protected void SetCheckboxSettings(Widget widget, string compName, TW_SchemaBasic base)
	{
		if(!widget) return;
		
		SCR_CheckboxComponent checkboxComp = SCR_CheckboxComponent.Cast(widget.FindHandler(SCR_CheckboxComponent));
		
		if(!checkboxComp) return;
		checkboxComp.SetLabel(base.title);
		
		if(base.defaultValue)
			checkboxComp.SetChecked(base.defaultValue == "true");
		
		checkboxComp.ComponentKey = compName;
	}
	
	protected void SetSliderSettings(Widget widget, TW_SchemaBasic base, TW_SchemaUI_Slider slider)
	{
		if(!widget) return;
		
		SCR_SliderComponent sliderComp = SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));
		
		sliderComp.SetMin(slider.min);
		
		if(slider.max > 0)
			sliderComp.SetMax(slider.max);
		
		if(slider.format)
			sliderComp.SetFormatText(slider.format);
		
		if(slider.step > 0)
			sliderComp.SetStep(slider.step);
		
		if(base.defaultValue)
		{
			float defaultValue = SCR_StringHelper.GetFloatsFromString(base.defaultValue)[0];
			sliderComp.SetValue(defaultValue);
		}
		
		sliderComp.SetLabel(base.title);
	}
	
	protected string GetWidgetTypeFor(string type)
	{
		switch(type)
		{
			case "integer":
			{
				return WIDGET_INTEGER_PREFAB;
			}
			
			case "float":
			{
				return WIDGET_FLOAT_PREFAB;
			}
			
			case "bool":
			{
				return WIDGET_BOOL_PREFAB;
			}
			
			case "string":
			{
				return WIDGET_STRING_PREFAB;
			}
			
			case "enum":
			{
				return WIDGET_ENUM_PREFAB;
			}
			
			case "flags":
			{
				return WIDGET_FLAGS_PREFAB;
			}
		}
		
		return WIDGET_STRING_PREFAB;
	}
}