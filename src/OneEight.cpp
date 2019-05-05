
#include "OneEight.hpp"

void OneEight::onReset() {
	_step = 0;
	_clock.reset();
	_reset.reset();
}

void OneEight::onSampleRateChange() {
	_timer.setParams(engineGetSampleRate(), 0.001f);
}

void OneEight::step() {
	bool reset = _reset.process(inputs[RESET_INPUT].value);
	if (reset) {
		_timer.reset();
	}
	bool timer = _timer.next();
	bool clock = _clock.process(inputs[CLOCK_INPUT].value) && !timer;

	int steps = clamp(params[STEPS_PARAM].value, 2.0f, 8.0f);
	int reverse = 1 - 2 * (params[DIRECTION_PARAM].value == 0.0f);
	_step = (_step + reverse * clock) % steps;
	_step += (_step < 0) * steps;
	_step -= _step * reset;
	int step = _step + (int)params[SELECT_PARAM].value;
	step += (int)(clamp(inputs[SELECT_INPUT].value, 0.0f, 10.0f) * 0.1f * 8.0f);
	step = step % 8;

	float in = inputs[IN_INPUT].value + !inputs[IN_INPUT].active * 10.0f;
	for (int i = 0; i < 8; ++i) {
		outputs[OUT1_OUTPUT + i].value = (step == i) * in;
		lights[OUT1_LIGHT + i].value = step == i;
	}
}

struct OneEightWidget : ModuleWidget {
	static constexpr int hp = 6;

	OneEightWidget(OneEight* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/OneEight.svg")));
			addChild(panel);
		}

		addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto stepsParamPosition = Vec(15.5, 131.5);
		auto directionParamPosition = Vec(16.0, 167.5);
		auto selectParamPosition = Vec(9.0, 230.0);

		auto clockInputPosition = Vec(11.5, 35.0);
		auto resetInputPosition = Vec(11.5, 72.0);
		auto selectInputPosition = Vec(11.5, 270.0);
		auto inInputPosition = Vec(11.5, 324.0);

		auto out1OutputPosition = Vec(54.5, 35.0);
		auto out2OutputPosition = Vec(54.5, 76.3);
		auto out3OutputPosition = Vec(54.5, 118.6);
		auto out4OutputPosition = Vec(54.5, 158.9);
		auto out5OutputPosition = Vec(54.5, 200.1);
		auto out6OutputPosition = Vec(54.5, 241.4);
		auto out7OutputPosition = Vec(54.5, 282.7);
		auto out8OutputPosition = Vec(54.5, 324.0);

		auto out1LightPosition = Vec(66.5, 61.5);
		auto out2LightPosition = Vec(66.5, 102.8);
		auto out3LightPosition = Vec(66.5, 145.1);
		auto out4LightPosition = Vec(66.5, 185.4);
		auto out5LightPosition = Vec(66.5, 226.6);
		auto out6LightPosition = Vec(66.5, 267.9);
		auto out7LightPosition = Vec(66.5, 309.2);
		auto out8LightPosition = Vec(66.5, 350.5);
		// end generated by svg_widgets.rb

		{
			auto w = ParamWidget::create<Knob16>(stepsParamPosition, module, OneEight::STEPS_PARAM, 2.0, 8.0, 8.0);
			dynamic_cast<Knob*>(w)->snap = true;
			addParam(w);
		}
		addParam(ParamWidget::create<SliderSwitch2State14>(directionParamPosition, module, OneEight::DIRECTION_PARAM, 0.0, 1.0, 1.0));
		{
			auto w = ParamWidget::create<Knob29>(selectParamPosition, module, OneEight::SELECT_PARAM, 0.0, 7.0, 0.0);
			dynamic_cast<Knob*>(w)->snap = true;
			addParam(w);
		}

		addInput(Port::create<Port24>(clockInputPosition, Port::INPUT, module, OneEight::CLOCK_INPUT));
		addInput(Port::create<Port24>(resetInputPosition, Port::INPUT, module, OneEight::RESET_INPUT));
		addInput(Port::create<Port24>(selectInputPosition, Port::INPUT, module, OneEight::SELECT_INPUT));
		addInput(Port::create<Port24>(inInputPosition, Port::INPUT, module, OneEight::IN_INPUT));

		addOutput(Port::create<Port24>(out1OutputPosition, Port::OUTPUT, module, OneEight::OUT1_OUTPUT));
		addOutput(Port::create<Port24>(out2OutputPosition, Port::OUTPUT, module, OneEight::OUT2_OUTPUT));
		addOutput(Port::create<Port24>(out3OutputPosition, Port::OUTPUT, module, OneEight::OUT3_OUTPUT));
		addOutput(Port::create<Port24>(out4OutputPosition, Port::OUTPUT, module, OneEight::OUT4_OUTPUT));
		addOutput(Port::create<Port24>(out5OutputPosition, Port::OUTPUT, module, OneEight::OUT5_OUTPUT));
		addOutput(Port::create<Port24>(out6OutputPosition, Port::OUTPUT, module, OneEight::OUT6_OUTPUT));
		addOutput(Port::create<Port24>(out7OutputPosition, Port::OUTPUT, module, OneEight::OUT7_OUTPUT));
		addOutput(Port::create<Port24>(out8OutputPosition, Port::OUTPUT, module, OneEight::OUT8_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(out1LightPosition, module, OneEight::OUT1_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(out2LightPosition, module, OneEight::OUT2_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(out3LightPosition, module, OneEight::OUT3_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(out4LightPosition, module, OneEight::OUT4_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(out5LightPosition, module, OneEight::OUT5_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(out6LightPosition, module, OneEight::OUT6_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(out7LightPosition, module, OneEight::OUT7_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(out8LightPosition, module, OneEight::OUT8_LIGHT));
	}
};

Model* modelOneEight = createModel<OneEight, OneEightWidget>("Bogaudio-OneEight", "1:8", "mux & sequential switch", SWITCH_TAG);
