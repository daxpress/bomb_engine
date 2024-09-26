#include "native_script.h"

#include "internal.h"

namespace BE_NAMESPACE
{
void NativeScript::start() { Log(ScriptCategory, LogSeverity::Display, "Starting Script"); }
void NativeScript::update(float tick) {}
}  // namespace BE_NAMESPACE