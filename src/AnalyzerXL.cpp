
#include "string.h"

#include "AnalyzerXL.hpp"

#define RANGE_KEY "range"
#define RANGE_DB_KEY "range_db"
#define SMOOTH_KEY "smooth"
#define QUALITY_KEY "quality"
#define QUALITY_GOOD_KEY "good"
#define QUALITY_HIGH_KEY "high"
#define QUALITY_ULTRA_KEY "ultra"
#define WINDOW_KEY "window"
#define WINDOW_NONE_KEY "none"
#define WINDOW_HAMMING_KEY "hamming"
#define WINDOW_KAISER_KEY "kaiser"

void AnalyzerXL::reset() {
	_core.resetChannels();
}

void AnalyzerXL::sampleRateChange() {
	_core.resetChannels();
}

json_t* AnalyzerXL::toJson(json_t* root) {
	json_object_set_new(root, RANGE_KEY, json_real(_range));
	json_object_set_new(root, RANGE_DB_KEY, json_real(_rangeDb));
	json_object_set_new(root, SMOOTH_KEY, json_real(_smooth));
	switch (_quality) {
		case AnalyzerCore::QUALITY_GOOD: {
			json_object_set_new(root, QUALITY_KEY, json_string(QUALITY_GOOD_KEY));
			break;
		}
		case AnalyzerCore::QUALITY_HIGH: {
			json_object_set_new(root, QUALITY_KEY, json_string(QUALITY_HIGH_KEY));
			break;
		}
		case AnalyzerCore::QUALITY_ULTRA: {
			json_object_set_new(root, QUALITY_KEY, json_string(QUALITY_ULTRA_KEY));
			break;
		}
	}
	switch (_window) {
		case AnalyzerCore::WINDOW_NONE: {
			json_object_set_new(root, WINDOW_KEY, json_string(WINDOW_NONE_KEY));
			break;
		}
		case AnalyzerCore::WINDOW_HAMMING: {
			json_object_set_new(root, WINDOW_KEY, json_string(WINDOW_HAMMING_KEY));
			break;
		}
		case AnalyzerCore::WINDOW_KAISER: {
			json_object_set_new(root, WINDOW_KEY, json_string(WINDOW_KAISER_KEY));
			break;
		}
	}
	return root;
}

void AnalyzerXL::fromJson(json_t* root) {
	json_t* jr = json_object_get(root, RANGE_KEY);
	if (jr) {
		_range = clamp(json_real_value(jr), -0.9f, 0.8f);
	}

	json_t* jrd = json_object_get(root, RANGE_DB_KEY);
	if (jrd) {
		_rangeDb = clamp(json_real_value(jrd), 80.0f, 140.0f);
	}

	json_t* js = json_object_get(root, SMOOTH_KEY);
	if (js) {
		_smooth = clamp(json_real_value(js), 0.0f, 0.5f);
	}

	json_t* jq = json_object_get(root, QUALITY_KEY);
	if (jq) {
		const char *s = json_string_value(jq);
		if (strcmp(s, QUALITY_GOOD_KEY) == 0) {
			_quality = AnalyzerCore::QUALITY_GOOD;
		}
		else if (strcmp(s, QUALITY_HIGH_KEY) == 0) {
			_quality = AnalyzerCore::QUALITY_HIGH;
		}
		else if (strcmp(s, QUALITY_ULTRA_KEY) == 0) {
			_quality = AnalyzerCore::QUALITY_ULTRA;
		}
	}

	json_t* jw = json_object_get(root, WINDOW_KEY);
	if (jw) {
		const char *s = json_string_value(jw);
		if (strcmp(s, WINDOW_NONE_KEY) == 0) {
			_window = AnalyzerCore::WINDOW_NONE;
		}
		else if (strcmp(s, WINDOW_HAMMING_KEY) == 0) {
			_window = AnalyzerCore::WINDOW_HAMMING;
		}
		else if (strcmp(s, WINDOW_KAISER_KEY) == 0) {
			_window = AnalyzerCore::WINDOW_KAISER;
		}
	}
}

void AnalyzerXL::modulate() {
	_rangeMinHz = 0.0f;
	_rangeMaxHz = 0.5f * APP->engine->getSampleRate();
	if (_range < 0.0f) {
		_rangeMaxHz *= 1.0f + _range;
	}
	else if (_range > 0.0f) {
		_rangeMinHz = _range * _rangeMaxHz;
	}

	float smooth = _smooth / (_core.size() / (_core._overlap * APP->engine->getSampleRate()));
	int averageN = std::max(1, (int)roundf(smooth));
	_core.setParams(averageN, _quality, _window);
}

void AnalyzerXL::processChannel(const ProcessArgs& args, int _c) {
	for (int i = 0; i < 8; ++i) {
		_core.stepChannel(i, inputs[SIGNALA_INPUT + i]);
	}
}

struct AnalyzerXLWidget : BGModuleWidget {
	static constexpr int hp = 42;

	AnalyzerXLWidget(AnalyzerXL* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
		setPanel(box.size, "AnalyzerXL", false);

		{
			auto inset = Vec(30, 1);
			auto size = Vec(box.size.x - inset.x - 1, 378);
			auto display = new AnalyzerDisplay(module, size, false);
			display->box.pos = inset;
			display->box.size = size;
			addChild(display);
		}

		// generated by svg_widgets.rb
		auto signalaInputPosition = Vec(3.0, 13.0);
		auto signalbInputPosition = Vec(3.0, 47.0);
		auto signalcInputPosition = Vec(3.0, 81.0);
		auto signaldInputPosition = Vec(3.0, 115.0);
		auto signaleInputPosition = Vec(3.0, 149.0);
		auto signalfInputPosition = Vec(3.0, 183.0);
		auto signalgInputPosition = Vec(3.0, 217.0);
		auto signalhInputPosition = Vec(3.0, 251.0);
		// end generated by svg_widgets.rb

		addInput(createInput<Port24>(signalaInputPosition, module, AnalyzerXL::SIGNALA_INPUT));
		addInput(createInput<Port24>(signalbInputPosition, module, AnalyzerXL::SIGNALB_INPUT));
		addInput(createInput<Port24>(signalcInputPosition, module, AnalyzerXL::SIGNALC_INPUT));
		addInput(createInput<Port24>(signaldInputPosition, module, AnalyzerXL::SIGNALD_INPUT));
		addInput(createInput<Port24>(signaleInputPosition, module, AnalyzerXL::SIGNALE_INPUT));
		addInput(createInput<Port24>(signalfInputPosition, module, AnalyzerXL::SIGNALF_INPUT));
		addInput(createInput<Port24>(signalgInputPosition, module, AnalyzerXL::SIGNALG_INPUT));
		addInput(createInput<Port24>(signalhInputPosition, module, AnalyzerXL::SIGNALH_INPUT));
	}

	void contextMenu(Menu* menu) override {
		auto a = dynamic_cast<AnalyzerXL*>(module);
		assert(a);

		menu->addChild(new MenuLabel());
		{
			OptionsMenuItem* mi = new OptionsMenuItem("Frequency range");
			mi->addItem(OptionMenuItem("Lower 25%", [a]() { return a->_range == -0.75f; }, [a]() { a->_range = -0.75f; }));
			mi->addItem(OptionMenuItem("Lower 50%", [a]() { return a->_range == -0.5f; }, [a]() { a->_range = -0.5f; }));
			mi->addItem(OptionMenuItem("Full", [a]() { return a->_range == 0.0f; }, [a]() { a->_range = 0.0f; }));
			mi->addItem(OptionMenuItem("Upper 50%", [a]() { return a->_range == 0.5f; }, [a]() { a->_range = 0.5f; }));
			mi->addItem(OptionMenuItem("Upper 25%", [a]() { return a->_range == 0.75f; }, [a]() { a->_range = 0.75f; }));
			OptionsMenuItem::addToMenu(mi, menu);
		}
		{
			OptionsMenuItem* mi = new OptionsMenuItem("Amplitude range");
			mi->addItem(OptionMenuItem("To -60dB", [a]() { return a->_rangeDb == 80.0f; }, [a]() { a->_rangeDb = 80.0f; }));
			mi->addItem(OptionMenuItem("To -120dB", [a]() { return a->_rangeDb == 140.0f; }, [a]() { a->_rangeDb = 140.0f; }));
			OptionsMenuItem::addToMenu(mi, menu);
		}
		{
			OptionsMenuItem* mi = new OptionsMenuItem("Smoothing");
			mi->addItem(OptionMenuItem("None", [a]() { return a->_smooth == 0.0f; }, [a]() { a->_smooth = 0.0f; }));
			mi->addItem(OptionMenuItem("10ms", [a]() { return a->_smooth == 0.01f; }, [a]() { a->_smooth = 0.01f; }));
			mi->addItem(OptionMenuItem("50ms", [a]() { return a->_smooth == 0.05f; }, [a]() { a->_smooth = 0.05f; }));
			mi->addItem(OptionMenuItem("100ms", [a]() { return a->_smooth == 0.1f; }, [a]() { a->_smooth = 0.1f; }));
			mi->addItem(OptionMenuItem("250ms", [a]() { return a->_smooth == 0.25f; }, [a]() { a->_smooth = 0.25f; }));
			mi->addItem(OptionMenuItem("500ms", [a]() { return a->_smooth == 0.5f; }, [a]() { a->_smooth = 0.5f; }));
			OptionsMenuItem::addToMenu(mi, menu);
		}
		{
			OptionsMenuItem* mi = new OptionsMenuItem("Quality");
			mi->addItem(OptionMenuItem("Good", [a]() { return a->_quality == AnalyzerCore::QUALITY_GOOD; }, [a]() { a->_quality = AnalyzerCore::QUALITY_GOOD; }));
			mi->addItem(OptionMenuItem("High", [a]() { return a->_quality == AnalyzerCore::QUALITY_HIGH; }, [a]() { a->_quality = AnalyzerCore::QUALITY_HIGH; }));
			mi->addItem(OptionMenuItem("Ultra", [a]() { return a->_quality == AnalyzerCore::QUALITY_ULTRA; }, [a]() { a->_quality = AnalyzerCore::QUALITY_ULTRA; }));
			OptionsMenuItem::addToMenu(mi, menu);
		}
		{
			OptionsMenuItem* mi = new OptionsMenuItem("Window");
			mi->addItem(OptionMenuItem("Kaiser", [a]() { return a->_window == AnalyzerCore::WINDOW_KAISER; }, [a]() { a->_window = AnalyzerCore::WINDOW_KAISER; }));
			mi->addItem(OptionMenuItem("Hamming", [a]() { return a->_window == AnalyzerCore::WINDOW_HAMMING; }, [a]() { a->_window = AnalyzerCore::WINDOW_HAMMING; }));
			mi->addItem(OptionMenuItem("None", [a]() { return a->_window == AnalyzerCore::WINDOW_NONE; }, [a]() { a->_window = AnalyzerCore::WINDOW_NONE; }));
			OptionsMenuItem::addToMenu(mi, menu);
		}
	}
};

Model* modelAnalyzerXL = bogaudio::createModel<AnalyzerXL, AnalyzerXLWidget>("Bogaudio-AnalyzerXL", "ANALYZER-XL", "8-channel spectrum analyzer", "Visual");
