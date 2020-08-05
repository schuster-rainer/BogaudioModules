
#include "PEQ14XV.hpp"
#include "dsp/pitch.hpp"

PEQ14XV::Engine::Engine() {
	filters[0] = new MultimodeFilter8();
	for (int i = 1; i < 13; ++i) {
		filters[i] = new MultimodeFilter4();
	}
	filters[13] = new MultimodeFilter8();
}

PEQ14XV::Engine::~Engine() {
	for (int i = 0; i < 14; ++i) {
		delete filters[i];
	}
}

void PEQ14XV::addChannel(int c) {
	_engines[c] = new Engine();
}

void PEQ14XV::removeChannel(int c) {
	delete _engines[c];
	_engines[c] = NULL;
}

void PEQ14XV::modulate() {
	float outGain = clamp(params[OUTPUT_GAIN_PARAM].getValue(), 0.0f, 1.0f);
	outGain *= maxOutputGain;
	_outputGain.setLevel(outGain);

	float b14Mix = clamp(params[BAND14_MIX_PARAM].getValue(), 0.0f, 1.0f);
	b14Mix = 1.0f - b14Mix;
	b14Mix *= Amplifier::minDecibels;
	_band14Mix.setLevel(b14Mix);

	_band1Enable = params[BAND1_ENABLE_PARAM].getValue() > 0.5f;
	_band14Enable = params[BAND14_ENABLE_PARAM].getValue() > 0.5f;
}

void PEQ14XV::modulateChannel(int c) {
	Engine& e = *_engines[c];

	float sr = APP->engine->getSampleRate();
	float response = sensitivity(params[EF_DAMP_PARAM], &inputs[EF_DAMP_INPUT], c);
	if (e.response != response) {
		e.response = response;
		for (int i = 0; i < 14; ++i) {
			e.efs[i].setParams(sr, e.response);
		}
	}

	e.efGain.setLevel(gain(params[EF_GAIN_PARAM], &inputs[EF_GAIN_INPUT], c));

	float transpose = clamp(params[TRANSPOSE_PARAM].getValue(), -1.0f, 1.0f);
	if (inputs[TRANSPOSE_INPUT].isConnected()) {
		transpose *= clamp(inputs[TRANSPOSE_INPUT].getPolyVoltage(c) / 5.0f, -1.0f, 1.0f);
	}
	e.transposeSemitones = 24.0f * transpose;
}

void PEQ14XV::processAlways(const ProcessArgs& args) {
	outputs[OUT_OUTPUT].setChannels(_channels);
	outputs[ODDS_OUTPUT].setChannels(_channels);
	outputs[EVENS_OUTPUT].setChannels(_channels);

	_baseMessage = NULL;
	if (baseConnected()) {
		_baseMessage = fromBase();
	}

	if (expanderConnected()) {
		if (_baseMessage) {
			// *toExpander() = *_baseMessage;
			_baseMessage->copyTo(toExpander());
		}
		else {
			toExpander()->reset();
		}
	}
}

void PEQ14XV::processChannel(const ProcessArgs& args, int c) {
	if (_baseMessage && _baseMessage->valid) {
		Engine& e = *_engines[c];
		float in = inputs[IN_INPUT].getPolyVoltage(c);
		float out = 0.0f;
		float odds = 0.0f;
		float evens = 0.0f;
		for (int i = 0; i < 14; ++i) {
			auto mode = MultimodeFilter::BANDPASS_MODE;
			int poles = 4;
			float bandwidth = _baseMessage->bandwidths[c];
			if (i == 0 && _baseMessage->lowLP) {
				mode = MultimodeFilter::LOWPASS_MODE;
				poles = 12;
				bandwidth = MultimodeFilter::minQbw;
			}
			if (i == 13 && _baseMessage->highHP) {
				mode = MultimodeFilter::HIGHPASS_MODE;
				poles = 12;
				bandwidth = MultimodeFilter::minQbw;
			}
			float f = _baseMessage->frequencies[c][i];
			if (e.transposeSemitones > 0.01f || e.transposeSemitones < -0.01f) {
				f = frequencyToSemitone(f);
				f += e.transposeSemitones;
				f = semitoneToFrequency(f);
			}
			if (f < MultimodeFilter::minFrequency || f > MultimodeFilter::maxFrequency) {
				continue;
			}

			e.filters[i]->setParams(
				APP->engine->getSampleRate(),
				MultimodeFilter::BUTTERWORTH_TYPE,
				poles,
				mode,
				f,
				bandwidth,
				MultimodeFilter::PITCH_BANDWIDTH_MODE
			);

			float level = e.efs[i].next(_baseMessage->outs[c][i]);
			level = scaleEF(level, _baseMessage->frequencies[c][i], _baseMessage->bandwidths[c]);
			level = e.efGain.next(level);
			level *= 0.1f;
			level = std::max(0.0f, std::min(1.0f, level));
			level = 1.0f - level;
			level *= Amplifier::minDecibels;
			e.amplifiers[i].setLevel(level);

			float o = e.amplifiers[i].next(e.filters[i]->next(in));
			o = _outputGain.next(o);
			out += (float)((i != 0 || _band1Enable) && (i != 13 || _band14Enable)) * o;
			odds += (float)(i % 2 == 0 && (i != 0 || _band1Enable)) * o;
			evens += (float)(i % 2 == 1 && (i != 13 || _band14Enable)) * o;
		}

		float band14raw = _baseMessage->outs[c][13];
		band14raw = _band14Mix.next(band14raw);
		out += band14raw;
		odds += band14raw;
		evens += band14raw;

		outputs[OUT_OUTPUT].setVoltage(_saturator.next(out), c);
		outputs[ODDS_OUTPUT].setVoltage(_saturator.next(odds), c);
		outputs[EVENS_OUTPUT].setVoltage(_saturator.next(evens), c);
	}
	else {
		outputs[OUT_OUTPUT].setVoltage(0.0f, c);
		outputs[ODDS_OUTPUT].setVoltage(0.0f, c);
		outputs[EVENS_OUTPUT].setVoltage(0.0f, c);
	}
}

struct PEQ14XVWidget : BGModuleWidget {
	static constexpr int hp = 5;

	PEQ14XVWidget(PEQ14XV* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PEQ14XV.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto efDampParamPosition = Vec(12.0, 40.0);
		auto efGainParamPosition = Vec(47.0, 40.0);
		auto transposeParamPosition = Vec(24.5, 132.0);
		auto outputGainParamPosition = Vec(12.0, 227.0);
		auto band14MixParamPosition = Vec(47.0, 227.0);
		auto band1EnableParamPosition = Vec(19.5, 262.0);
		auto band14EnableParamPosition = Vec(57.0, 262.0);

		auto efDampInputPosition = Vec(8.0, 73.0);
		auto efGainInputPosition = Vec(43.0, 73.0);
		auto transposeInputPosition = Vec(25.5, 169.0);
		auto inInputPosition = Vec(10.5, 290.0);

		auto oddsOutputPosition = Vec(40.5, 290.0);
		auto outOutputPosition = Vec(10.5, 325.0);
		auto evensOutputPosition = Vec(40.5, 325.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob16>(efDampParamPosition, module, PEQ14XV::EF_DAMP_PARAM));
		addParam(createParam<Knob16>(efGainParamPosition, module, PEQ14XV::EF_GAIN_PARAM));
		addParam(createParam<Knob26>(transposeParamPosition, module, PEQ14XV::TRANSPOSE_PARAM));
		addParam(createParam<Knob16>(outputGainParamPosition, module, PEQ14XV::OUTPUT_GAIN_PARAM));
		addParam(createParam<Knob16>(band14MixParamPosition, module, PEQ14XV::BAND14_MIX_PARAM));
		addParam(createParam<IndicatorButtonGreen9>(band1EnableParamPosition, module, PEQ14XV::BAND1_ENABLE_PARAM));
		addParam(createParam<IndicatorButtonGreen9>(band14EnableParamPosition, module, PEQ14XV::BAND14_ENABLE_PARAM));

		addInput(createInput<Port24>(efDampInputPosition, module, PEQ14XV::EF_DAMP_INPUT));
		addInput(createInput<Port24>(efGainInputPosition, module, PEQ14XV::EF_GAIN_INPUT));
		addInput(createInput<Port24>(transposeInputPosition, module, PEQ14XV::TRANSPOSE_INPUT));
		addInput(createInput<Port24>(inInputPosition, module, PEQ14XV::IN_INPUT));

		addOutput(createOutput<Port24>(oddsOutputPosition, module, PEQ14XV::ODDS_OUTPUT));
		addOutput(createOutput<Port24>(outOutputPosition, module, PEQ14XV::OUT_OUTPUT));
		addOutput(createOutput<Port24>(evensOutputPosition, module, PEQ14XV::EVENS_OUTPUT));
	}
};

Model* modelPEQ14XV = createModel<PEQ14XV, PEQ14XVWidget>("Bogaudio-PEQ14XV", "PEQ14XV", "PEQ14 vocoder expander", "Filter", "Vocoder", "Expander", "Polyphonic");
