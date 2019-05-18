
#include "Noise.hpp"

void Noise::process(const ProcessArgs& args) {
	if (outputs[BLUE_OUTPUT].isConnected()) {
		outputs[BLUE_OUTPUT].setVoltage(clamp(_blue.next() * 20.0f, -10.0f, 10.f));
	}
	if (outputs[WHITE_OUTPUT].isConnected()) {
		outputs[WHITE_OUTPUT].setVoltage(clamp(_white.next() * 10.0f, -10.0f, 10.f));
	}
	if (outputs[PINK_OUTPUT].isConnected()) {
		outputs[PINK_OUTPUT].setVoltage(clamp(_pink.next() * 15.0f, -10.0f, 10.f));
	}
	if (outputs[RED_OUTPUT].isConnected()) {
		outputs[RED_OUTPUT].setVoltage(clamp(_red.next() * 20.0f, -10.0f, 10.f));
	}
	if (outputs[GAUSS_OUTPUT].isConnected()) {
		outputs[GAUSS_OUTPUT].setVoltage(clamp(_gauss.next(), -10.0f, 10.f));
	}

	float in = 0.0;
	if (inputs[ABS_INPUT].isConnected()) {
		in = inputs[ABS_INPUT].getVoltage();
		if (in < 0.0) {
			in = -in;
		}
	}
	outputs[ABS_OUTPUT].setVoltage(in);
}

struct NoiseWidget : ModuleWidget {
	static constexpr int hp = 3;

	NoiseWidget(Noise* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Noise.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto absInputPosition = Vec(10.5, 253.0);

		auto blueOutputPosition = Vec(10.5, 24.0);
		auto whiteOutputPosition = Vec(10.5, 65.0);
		auto pinkOutputPosition = Vec(10.5, 106.0);
		auto redOutputPosition = Vec(10.5, 147.0);
		auto gaussOutputPosition = Vec(10.5, 188.0);
		auto absOutputPosition = Vec(10.5, 291.0);
		// end generated by svg_widgets.rb

		addInput(createInput<Port24>(absInputPosition, module, Noise::ABS_INPUT));

		addOutput(createOutput<Port24>(blueOutputPosition, module, Noise::BLUE_OUTPUT));
		addOutput(createOutput<Port24>(whiteOutputPosition, module, Noise::WHITE_OUTPUT));
		addOutput(createOutput<Port24>(pinkOutputPosition, module, Noise::PINK_OUTPUT));
		addOutput(createOutput<Port24>(redOutputPosition, module, Noise::RED_OUTPUT));
		addOutput(createOutput<Port24>(gaussOutputPosition, module, Noise::GAUSS_OUTPUT));
		addOutput(createOutput<Port24>(absOutputPosition, module, Noise::ABS_OUTPUT));
	}
};

Model* modelNoise = bogaudio::createModel<Noise, NoiseWidget>("Bogaudio-Noise", "Noise",  "noise source", NOISE_TAG, RANDOM_TAG);
