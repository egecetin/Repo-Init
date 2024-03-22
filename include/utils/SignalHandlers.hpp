#pragma once

#include <csignal>
#include <functional>
#include <map>

/**
 * @brief A class that provides utility functions for handling signals.
 */
class SignalHandlers {
  private:
	static std::map<int, std::function<void(int)>> signalHandlers;

	/**
	 * @brief Internal signal handler function that invokes the registered signal handler for a given signal.
	 * @param[in] signal The signal number.
	 */
	static void signalHandler(int signal)
	{
		if (signalHandlers.find(signal) != signalHandlers.end())
		{
			signalHandlers[signal](signal);
		}
	}

  public:
	SignalHandlers() = delete;

	/**
	 * @brief Sets a signal handler for a specific signal.
	 * @param[in] signal The signal number.
	 * @param[in] handler The signal handler function.
	 */
	static bool setSignalHandler(int signal, std::function<void(int)> handler)
	{
		signalHandlers[signal] = handler;
		return std::signal(signal, SignalHandlers::signalHandler) != SIG_ERR;
	}

	/**
	 * @brief Removes the signal handler for a specific signal.
	 * @param[in] signal The signal number.
	 */
	static bool removeSignalHandler(int signal)
	{
		signalHandlers.erase(signal);
		return std::signal(signal, SIG_DFL) != SIG_ERR;
	}

	/**
	 * @brief Removes all registered signal handlers.
	 */
	static void removeSignalHandlers()
	{
		for (auto &handler : signalHandlers)
		{
			std::signal(handler.first, SIG_DFL);
		}
		signalHandlers.clear();
	}

	/**
	 * @brief Ignores a specific signal.
	 * @param[in] signal The signal number.
	 */
	static bool ignoreSignal(int signal) { return std::signal(signal, SIG_IGN) != SIG_ERR; }

	/**
	 * @brief Ignores multiple signals.
	 * @param[in] signals A vector of signal numbers.
	 */
	static void ignoreSignals(const std::vector<int> &signals)
	{
		for (auto &signal : signals)
		{
			std::signal(signal, SIG_IGN);
		}
	}

	/**
	 * @brief Restores the default behavior for a specific signal.
	 * @param[in] signal The signal number.
	 */
	static bool restoreSignal(int signal) { return std::signal(signal, SIG_DFL) != SIG_ERR; }

	/**
	 * @brief Restores the default behavior for multiple signals.
	 * @param[in] signals A vector of signal numbers.
	 */
	static void restoreSignals(const std::vector<int> &signals)
	{
		for (auto &signal : signals)
		{
			std::signal(signal, SIG_DFL);
		}
	}

	/**
	 * @brief Restores the default behavior for all signals.
	 */
	static void restoreAllSignals()
	{
		for (int signal = 1; signal < 32; signal++)
		{
			std::signal(signal, SIG_DFL);
		}
	}
};
