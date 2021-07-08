#include <iostream>
#include <thread>
#include <string>

#include "RtMidi.h"
#include "midi_input.h"


void message_callback(double timeStamp, std::vector<unsigned char> *message, void *userData) {
	MidiInput *self = static_cast<MidiInput *>(userData);

	if (message->size() >= 3) {
		self->total_time_since_start += timeStamp;

		if (self->cached_messages.size() < self->MAX_CACHED_MESSAGES) {
			Message mes(message, self->total_time_since_start, true);

			self->cached_messages.push_back(mes);
		}
	}
}

void MidiInput::start_input_system(int port) { // only ever call once

	// start loop here
	midiin = std::make_unique<RtMidiIn>();

	total_time_since_start = 0;

	// check port
	unsigned int nPorts = midiin->getPortCount();
	if (nPorts < port + 1) {
		std::cout << "Port not available! " << std::endl;

		// disable functionality
		port_name = "null";
		is_opperating = false;
	} else { // only should happen if port exists
		port_name = midiin->getPortName(port).c_str();
		is_opperating = true;

		midiin->openPort(port);
		std::cout << "Reading MIDI from " << port_name.to_int() << "." << std::endl;
		midiin->setCallback(&message_callback, this);
	}
}

Array MidiInput::get_messages() {
	Array _cached_messages;


	if (is_opperating) {
		for (Message mes : cached_messages) {
			if (mes.Convert()[0] != "null") _cached_messages.append(mes.Convert()); // ignores non note inputs for now
		}

		cached_messages.clear();
	}

	return _cached_messages;
}

String MidiInput::get_port_name() {
	return port_name; // if "null" is returned no port is currently active
}

bool MidiInput::is_port_connected(int port) { // if returns false object needs to be deleted and instanced again - this function should be called at leastt every few seconds
	if (!is_opperating) {
		port_name = "null";
		is_opperating = false;
		return false;
	}

	unsigned int nPorts = midiin->getPortCount();
	if (nPorts < port + 1) {
		port_name = "null";
		is_opperating = false;
		return false;
	}

	return true;
}

// binds methods to class
void MidiInput::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start_input_system", "port"), &MidiInput::start_input_system);

	// requires input system to be started
	ClassDB::bind_method(D_METHOD("get_messages"), &MidiInput::get_messages);
	ClassDB::bind_method(D_METHOD("get_port_name"), &MidiInput::get_port_name);
	ClassDB::bind_method(D_METHOD("is_port_connected", "port"), &MidiInput::is_port_connected);
}


MidiInput::MidiInput() {

}

MidiInput::~MidiInput() {
}
