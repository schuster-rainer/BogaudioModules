
#include "Follow.hpp"

bool Follow::active() {
	return inputs[IN_INPUT].isConnected() && outputs[OUT_OUTPUT].isConnected();
}

int Follow::channels() {
	return inputs[IN_INPUT].getChannels();
}

void Follow::addChannel(int c) {
	_engines[c] = new Engine;
}

void Follow::removeChannel(int c) {
	delete _engines[c];
	_engines[c] = NULL;
}

void Follow::modulateChannel(int c) {
	Engine& e = *_engines[c];
	e.follower.setParams(APP->engine->getSampleRate(), sensitivity(params[RESPONSE_PARAM], &inputs[RESPONSE_INPUT], c));
	e.gain.setLevel(gain(params[GAIN_PARAM], &inputs[GAIN_INPUT], c));
}

void Follow::processChannel(const ProcessArgs& args, int c) {
	Engine& e = *_engines[c];
	outputs[OUT_OUTPUT].setChannels(_channels);
	outputs[OUT_OUTPUT].setVoltage(_saturator.next(e.gain.next(e.follower.next(inputs[IN_INPUT].getVoltage(c)))), c);
}

struct FollowWidget : BGModuleWidget {
	static constexpr int hp = 3;

	FollowWidget(Follow* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Follow.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto responseParamPosition = Vec(8.0, 36.0);
		auto gainParamPosition = Vec(8.0, 142.0);

		auto responseInputPosition = Vec(10.5, 77.0);
		auto gainInputPosition = Vec(10.5, 183.0);
		auto inInputPosition = Vec(10.5, 233.0);

		auto outOutputPosition = Vec(10.5, 271.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob29>(responseParamPosition, module, Follow::RESPONSE_PARAM));
		addParam(createParam<Knob29>(gainParamPosition, module, Follow::GAIN_PARAM));

		addInput(createInput<Port24>(responseInputPosition, module, Follow::RESPONSE_INPUT));
		addInput(createInput<Port24>(gainInputPosition, module, Follow::GAIN_INPUT));
		addInput(createInput<Port24>(inInputPosition, module, Follow::IN_INPUT));

		addOutput(createOutput<Port24>(outOutputPosition, module, Follow::OUT_OUTPUT));
	}
};

Model* modelFollow = bogaudio::createModel<Follow, FollowWidget>("Bogaudio-Follow", "FOLLOW", "Envelope follower", "Envelope follower", "Polyphonic");
