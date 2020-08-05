
#include "VCF.hpp"
#include "dsp/pitch.hpp"

#define BANDWIDTH_MODE_KEY "bandwidthMode"
#define LINEAR_BANDWIDTH_MODE_KEY "linear"
#define PITCH_BANDWIDTH_MODE_KEY "pitched"

void VCF::Engine::setParams(
	float slope,
	MultimodeFilter::Mode mode,
	float frequency,
	float qbw,
	MultimodeFilter::BandwidthMode bwm
) {
	frequency = semitoneToFrequency(_frequencySL.next(frequencyToSemitone(frequency)));

	int i = -1, j = -1;
	std::fill(_gains, _gains + nFilters, 0.0f);
	if (slope >= 1.0f) {
		_gains[i = nFilters - 1] = 1.0f;
	}
	else {
		slope *= nFilters - 1;
		float r = std::fmod(slope, 1.0f);
		_gains[i = slope] = 1.0f - r;
		_gains[j = i + 1] = r;
	}

	_filters[i].setParams(
		_sampleRate,
		MultimodeFilter::BUTTERWORTH_TYPE,
		i + 1,
		mode,
		frequency,
		qbw,
		bwm
	);
	if (j >= 0) {
		_filters[j].setParams(
			_sampleRate,
			MultimodeFilter::BUTTERWORTH_TYPE,
			j + 1,
			mode,
			frequency,
			qbw,
			bwm
		);
	}
}

void VCF::Engine::sampleRateChange() {
	_sampleRate = APP->engine->getSampleRate();
	_frequencySL.setParams(_sampleRate, 0.5f, frequencyToSemitone(maxFrequency - minFrequency));
	_finalHP.setParams(_sampleRate, MultimodeFilter::BUTTERWORTH_TYPE, 2, MultimodeFilter::HIGHPASS_MODE, 80.0f, MultimodeFilter::minQbw, MultimodeFilter::LINEAR_BANDWIDTH_MODE, MultimodeFilter::MINIMUM_DELAY_MODE);
	for (int i = 0; i < nFilters; ++i) {
		_gainSLs[i].setParams(_sampleRate, 50.0f, 1.0f);
	}
}

void VCF::Engine::reset() {
	for (int i = 0; i < nFilters; ++i) {
		_filters[i].reset();
	}
}

float VCF::Engine::next(float sample) {
	float out = 0.0f;
	for (int i = 0; i < nFilters; ++i) {
		float g = _gainSLs[i].next(_gains[i]);
		if (g > 0.0f) {
			out += g * _filters[i].next(sample);
		}
	}
	return _finalHP.next(out);
}

json_t* VCF::toJson(json_t* root) {
	switch (_bandwidthMode) {
		case MultimodeFilter::LINEAR_BANDWIDTH_MODE: {
			json_object_set_new(root, BANDWIDTH_MODE_KEY, json_string(LINEAR_BANDWIDTH_MODE_KEY));
			break;
		}
		case MultimodeFilter::PITCH_BANDWIDTH_MODE: {
			json_object_set_new(root, BANDWIDTH_MODE_KEY, json_string(PITCH_BANDWIDTH_MODE_KEY));
			break;
		}
		default: {}
	}
	return root;
}

void VCF::fromJson(json_t* root) {
	json_t* bwm = json_object_get(root, BANDWIDTH_MODE_KEY);
	if (bwm) {
		if (strcmp(json_string_value(bwm), LINEAR_BANDWIDTH_MODE_KEY) == 0) {
			_bandwidthMode = MultimodeFilter::LINEAR_BANDWIDTH_MODE;
		}
		else {
			_bandwidthMode = MultimodeFilter::PITCH_BANDWIDTH_MODE;
		}
	}
}

void VCF::sampleRateChange() {
	for (int c = 0; c < _channels; ++c) {
		_engines[c]->sampleRateChange();
	}
}

bool VCF::active() {
	return outputs[OUT_OUTPUT].isConnected();
}

int VCF::channels() {
	return inputs[IN_INPUT].getChannels();
}

void VCF::addChannel(int c) {
	_engines[c] = new Engine();
}

void VCF::removeChannel(int c) {
	delete _engines[c];
	_engines[c] = NULL;
}

void VCF::modulate() {
	MultimodeFilter::Mode mode = (MultimodeFilter::Mode)(1 + clamp((int)params[MODE_PARAM].getValue(), 0, 4));
	if (_mode != mode) {
		_mode = mode;
		for (int c = 0; c < _channels; ++c) {
			_engines[c]->reset();
		}
	}
}

void VCF::modulateChannel(int c) {
	Engine& e = *_engines[c];

	float slope = clamp(params[SLOPE_PARAM].getValue(), 0.0f, 1.0f);
	if (inputs[SLOPE_INPUT].isConnected()) {
		slope *= clamp(inputs[SLOPE_INPUT].getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
	}
	slope *= slope;

	float q = clamp(params[Q_PARAM].getValue(), 0.0f, 1.0f);
	if (inputs[Q_INPUT].isConnected()) {
		q *= clamp(inputs[Q_INPUT].getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
	}

	float f = clamp(params[FREQUENCY_PARAM].getValue(), 0.0f, 1.0f);
	if (inputs[FREQUENCY_CV_INPUT].isConnected()) {
		float fcv = clamp(inputs[FREQUENCY_CV_INPUT].getPolyVoltage(c) / 5.0f, -1.0f, 1.0f);
		fcv *= clamp(params[FREQUENCY_CV_PARAM].getValue(), -1.0f, 1.0f);
		f = std::max(0.0f, f + fcv);
	}
	f *= f;
	f *= maxFrequency;
	if (inputs[PITCH_INPUT].isConnected() || inputs[FM_INPUT].isConnected()) {
		float fm = inputs[FM_INPUT].getPolyVoltage(c);
		fm *= clamp(params[FM_PARAM].getValue(), 0.0f, 1.0f);
		float pitch = clamp(inputs[PITCH_INPUT].getPolyVoltage(c), -5.0f, 5.0f);
		pitch += fm;
		f += cvToFrequency(pitch);
	}
	f = clamp(f, minFrequency, maxFrequency);

	e.setParams(
		slope,
		_mode,
		f,
		q,
		_bandwidthMode
	);
}

void VCF::processAll(const ProcessArgs& args) {
	outputs[OUT_OUTPUT].setChannels(_channels);
}

void VCF::processChannel(const ProcessArgs& args, int c) {
	outputs[OUT_OUTPUT].setVoltage(_engines[c]->next(inputs[IN_INPUT].getVoltage(c)), c);
}

struct VCFWidget : BGModuleWidget {
	static constexpr int hp = 10;

	VCFWidget(VCF* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/VCF.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto frequencyParamPosition = Vec(41.0, 45.0);
		auto frequencyCvParamPosition = Vec(45.0, 138.0);
		auto fmParamPosition = Vec(102.0, 138.0);
		auto modeParamPosition = Vec(67.0, 176.0);
		auto qParamPosition = Vec(26.5, 220.0);
		auto slopeParamPosition = Vec(97.5, 220.0);

		auto frequencyCvInputPosition = Vec(31.0, 274.0);
		auto pitchInputPosition = Vec(63.0, 274.0);
		auto fmInputPosition = Vec(95.0, 274.0);
		auto inInputPosition = Vec(15.0, 318.0);
		auto qInputPosition = Vec(47.0, 318.0);
		auto slopeInputPosition = Vec(79.0, 318.0);

		auto outOutputPosition = Vec(111.0, 318.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob68>(frequencyParamPosition, module, VCF::FREQUENCY_PARAM));
		addParam(createParam<Knob16>(frequencyCvParamPosition, module, VCF::FREQUENCY_CV_PARAM));
		addParam(createParam<Knob16>(fmParamPosition, module, VCF::FM_PARAM));
		addParam(createParam<Knob38>(qParamPosition, module, VCF::Q_PARAM));
		{
			auto w = createParam<Knob16>(modeParamPosition, module, VCF::MODE_PARAM);
			auto k = dynamic_cast<SvgKnob*>(w);
			k->snap = true;
			float a = (22.5 / 180.0) * M_PI;
			k->minAngle = a;
			k->maxAngle = M_PI - a;
			k->speed = 3.0;
			addParam(w);
		}
		addParam(createParam<Knob38>(slopeParamPosition, module, VCF::SLOPE_PARAM));

		addInput(createInput<Port24>(frequencyCvInputPosition, module, VCF::FREQUENCY_CV_INPUT));
		addInput(createInput<Port24>(fmInputPosition, module, VCF::FM_INPUT));
		addInput(createInput<Port24>(pitchInputPosition, module, VCF::PITCH_INPUT));
		addInput(createInput<Port24>(inInputPosition, module, VCF::IN_INPUT));
		addInput(createInput<Port24>(qInputPosition, module, VCF::Q_INPUT));
		addInput(createInput<Port24>(slopeInputPosition, module, VCF::SLOPE_INPUT));

		addOutput(createOutput<Port24>(outOutputPosition, module, VCF::OUT_OUTPUT));
	}

	void contextMenu(Menu* menu) override {
		auto m = dynamic_cast<VCF*>(module);
		assert(m);
		menu->addChild(new MenuLabel());
		OptionsMenuItem* bwm = new OptionsMenuItem("Bandwidth mode");
		bwm->addItem(OptionMenuItem("Pitched", [m]() { return m->_bandwidthMode == MultimodeFilter::PITCH_BANDWIDTH_MODE; }, [m]() { m->_bandwidthMode = MultimodeFilter::PITCH_BANDWIDTH_MODE; }));
		bwm->addItem(OptionMenuItem("Linear", [m]() { return m->_bandwidthMode == MultimodeFilter::LINEAR_BANDWIDTH_MODE; }, [m]() { m->_bandwidthMode = MultimodeFilter::LINEAR_BANDWIDTH_MODE; }));
		OptionsMenuItem::addToMenu(bwm, menu);
	}
};

Model* modelVCF = createModel<VCF, VCFWidget>("Bogaudio-VCF", "VCF", "Multimode filter", "Filter", "Polyphonic");
