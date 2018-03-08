
#include "Test.hpp"

void Test::onReset() {
}

void Test::step() {
	if (!outputs[OUT_OUTPUT].active) {
		return;
	}

#ifdef LPF
	if (!inputs[IN_INPUT].active) {
		return;
	}
	_lpf.setParams(
		engineGetSampleRate(),
		10000.0 * clamp(params[PARAM1_PARAM].value, 0.0f, 1.0f),
		std::max(10.0 * clamp(params[PARAM2_PARAM].value, 0.0f, 1.0f), 0.1)
	);
	outputs[OUT_OUTPUT].value = _lpf.next(inputs[IN_INPUT].value);

#elif LPFNOISE
	_lpf.setParams(
		engineGetSampleRate(),
		22000.0 * clamp(params[PARAM1_PARAM].value, 0.0f, 1.0f),
		0.717f
	);
	float noise = _noise.next();
	outputs[OUT_OUTPUT].value = _lpf.next(noise) * 10.0;;
	outputs[OUT2_OUTPUT].value = noise * 10.0;;

#elif SINE
	_sine.setSampleRate(engineGetSampleRate());
	_sine.setFrequency(oscillatorPitch());
	outputs[OUT_OUTPUT].value = _sine.next();

	_sine2.setSampleRate(engineGetSampleRate());
	_sine2.setFrequency(oscillatorPitch());
	outputs[OUT2_OUTPUT].value = _sine2.next();

#elif SQUARE
	_square.setSampleRate(engineGetSampleRate());
	_square.setFrequency(oscillatorPitch());
	float pw = params[PARAM2_PARAM].value;
	if (inputs[CV2_INPUT].active) {
		pw += clamp(inputs[CV2_INPUT].value, -5.0f, 5.0f) / 10.0f;
	}
	_square.setPulseWidth(pw);
	outputs[OUT_OUTPUT].value = _square.next();

#elif SAW
	_saw.setSampleRate(engineGetSampleRate());
	_saw.setFrequency(oscillatorPitch());
	outputs[OUT_OUTPUT].value = _saw.next();

#elif TRIANGLE
	_triangle.setSampleRate(engineGetSampleRate());
	_triangle.setFrequency(oscillatorPitch());
	outputs[OUT_OUTPUT].value = _triangle.next();

#elif SINEBANK
	_sineBank.setSampleRate(engineGetSampleRate());
	_sineBank.setFrequency(oscillatorPitch());
	outputs[OUT_OUTPUT].value = _sineBank.next();

#elif OVERSAMPLING
	_saw1.setSampleRate(engineGetSampleRate());
	_saw1.setFrequency(oscillatorPitch() / (float)OVERSAMPLEN);
	float buf[OVERSAMPLEN];
	for (int i = 0; i < OVERSAMPLEN; ++i) {
		buf[i] = _saw1.next();
	}
	outputs[OUT_OUTPUT].value = _rackDecimator.process(buf);

	_saw2.setSampleRate(engineGetSampleRate());
	_saw2.setFrequency(oscillatorPitch() / (float)OVERSAMPLEN);
	_lpf.setParams(
		engineGetSampleRate(),
		engineGetSampleRate() / 4.0f,
		0.03
	);
	_lpf2.setParams(
		engineGetSampleRate(),
		engineGetSampleRate() / 4.0f,
		0.03
	);
	float s = 0.0f;
	for (int i = 0; i < OVERSAMPLEN; ++i) {
	  // s = _lpf2.next(_lpf.next(_saw2.next()));
		s = _lpf.next(_saw2.next());
		// s = _saw2.next();
	}
	outputs[OUT2_OUTPUT].value = s * 5.0;
#endif
}

float Test::oscillatorPitch() {
	if (inputs[CV1_INPUT].active) {
		return cvToFrequency(inputs[CV1_INPUT].value);
	}
	return 10000.0 * powf(params[PARAM1_PARAM].value, 2.0);
}


struct TestWidget : ModuleWidget {
	TestWidget(Test* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * 3, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Test.svg")));
			addChild(panel);
		}

		addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto param1ParamPosition = Vec(9.5, 28.5);
		auto param2ParamPosition = Vec(9.5, 128.5);

		auto cv1InputPosition = Vec(10.5, 66.0);
		auto cv2InputPosition = Vec(10.5, 168.0);
		auto inInputPosition = Vec(10.5, 258.0);

		auto outOutputPosition = Vec(10.5, 296.0);
		auto out2OutputPosition = Vec(20.5, 316.0);
		// end generated by svg_widgets.rb

		addParam(ParamWidget::create<Knob26>(param1ParamPosition, module, Test::PARAM1_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Knob26>(param2ParamPosition, module, Test::PARAM2_PARAM, 0.0, 1.0, 0.5));

		addInput(Port::create<Port24>(cv1InputPosition, Port::INPUT, module, Test::CV1_INPUT));
		addInput(Port::create<Port24>(cv2InputPosition, Port::INPUT, module, Test::CV2_INPUT));
		addInput(Port::create<Port24>(inInputPosition, Port::INPUT, module, Test::IN_INPUT));

		addOutput(Port::create<Port24>(outOutputPosition, Port::OUTPUT, module, Test::OUT_OUTPUT));
		addOutput(Port::create<Port24>(out2OutputPosition, Port::OUTPUT, module, Test::OUT2_OUTPUT));
	}
};

Model* modelTest = Model::create<Test, TestWidget>("Bogaudio", "Bogaudio-Test", "Test");
