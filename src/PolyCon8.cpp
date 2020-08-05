
#include "PolyCon8.hpp"

void PolyCon8::processAll(const ProcessArgs& args) {
	int cn = clamp(_polyChannels, 1, 8);

	outputs[OUT_OUTPUT].setChannels(cn);
	for (int c = 0; c < cn; ++c) {
		float out = clamp(params[CHANNEL1_PARAM + c].getValue(), -1.0f, 1.0f);
		out += _rangeOffset;
		out *= _rangeScale;
		outputs[OUT_OUTPUT].setVoltage(out, c);
		lights[CHANNEL1_LIGHT + c].value = 1.0f;
	}
	for (int c = cn; c < 8; ++c) {
		lights[CHANNEL1_LIGHT + c].value = 0.0f;
	}
}

struct PolyCon8Widget : BGModuleWidget {
	static constexpr int hp = 3;

	PolyCon8Widget(PolyCon8* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PolyCon8.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto channel1ParamPosition = Vec(14.5, 24.0);
		auto channel2ParamPosition = Vec(14.5, 55.5);
		auto channel3ParamPosition = Vec(14.5, 87.0);
		auto channel4ParamPosition = Vec(14.5, 118.5);
		auto channel5ParamPosition = Vec(14.5, 150.0);
		auto channel6ParamPosition = Vec(14.5, 181.5);
		auto channel7ParamPosition = Vec(14.5, 213.0);
		auto channel8ParamPosition = Vec(14.5, 244.5);

		auto outOutputPosition = Vec(10.5, 274.0);

		auto channel1LightPosition = Vec(38.5, 30.4);
		auto channel2LightPosition = Vec(38.5, 61.9);
		auto channel3LightPosition = Vec(38.5, 93.4);
		auto channel4LightPosition = Vec(38.5, 124.9);
		auto channel5LightPosition = Vec(38.5, 156.4);
		auto channel6LightPosition = Vec(38.5, 187.9);
		auto channel7LightPosition = Vec(38.5, 219.4);
		auto channel8LightPosition = Vec(38.5, 250.9);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob16>(channel1ParamPosition, module, PolyCon8::CHANNEL1_PARAM));
		addParam(createParam<Knob16>(channel2ParamPosition, module, PolyCon8::CHANNEL2_PARAM));
		addParam(createParam<Knob16>(channel3ParamPosition, module, PolyCon8::CHANNEL3_PARAM));
		addParam(createParam<Knob16>(channel4ParamPosition, module, PolyCon8::CHANNEL4_PARAM));
		addParam(createParam<Knob16>(channel5ParamPosition, module, PolyCon8::CHANNEL5_PARAM));
		addParam(createParam<Knob16>(channel6ParamPosition, module, PolyCon8::CHANNEL6_PARAM));
		addParam(createParam<Knob16>(channel7ParamPosition, module, PolyCon8::CHANNEL7_PARAM));
		addParam(createParam<Knob16>(channel8ParamPosition, module, PolyCon8::CHANNEL8_PARAM));

		addOutput(createOutput<Port24>(outOutputPosition, module, PolyCon8::OUT_OUTPUT));

		addChild(createLight<TinyLight<GreenLight>>(channel1LightPosition, module, PolyCon8::CHANNEL1_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(channel2LightPosition, module, PolyCon8::CHANNEL2_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(channel3LightPosition, module, PolyCon8::CHANNEL3_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(channel4LightPosition, module, PolyCon8::CHANNEL4_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(channel5LightPosition, module, PolyCon8::CHANNEL5_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(channel6LightPosition, module, PolyCon8::CHANNEL6_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(channel7LightPosition, module, PolyCon8::CHANNEL7_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(channel8LightPosition, module, PolyCon8::CHANNEL8_LIGHT));
	}

	void contextMenu(Menu* menu) override {
		auto m = dynamic_cast<PolyCon8*>(module);
		assert(m);
		menu->addChild(new MenuLabel());
		menu->addChild(new PolyChannelsMenuItem(m, 8));
		OutputRangeOptionMenuItem::addOutputRangeOptionsToMenu(module, menu);
	}
};

Model* modelPolyCon8 = createModel<PolyCon8, PolyCon8Widget>("Bogaudio-PolyCon8", "POLYCON8", "Polyphonic per-channel constant voltages", "Polyphonic");
