#pragma once
#include <string>

// Returns true ONLY after explicit confirmation via long-press GPIO24.
// The confirmation dialog has no side effects on the network interface.
bool confirmNetworkInterfaceChange(const std::string &interfaceName, bool enable);
