#pragma once

namespace Log
{
	/* Log to console with custom header.
	* @param txt Text to output.
	* @param header Header to use.
	*/
	void LogCustom(const char* txt, const char* header);

	/* Log to console with an info header.
	* @param txt Text to output.
	*/
	void LogInfo(const char* txt);

	/* Log to console with a warning header.
	* @param txt Text to output.
	*/
	void LogWarning(const char* txt);

	/* Log to console with an error header.
	* @param txt Text to output.
	*/
	void LogError(const char* txt);
}