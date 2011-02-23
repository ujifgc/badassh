/**
 * This file contains the bada application entry point.
 */
#include "SSHBada.h"

using namespace Osp::Base;
using namespace Osp::Base::Collection;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

_EXPORT_ int OspMain(int argc, char *pArgv[]);

/**
 * The entry function of bada application called by the operating system.
 */
int
OspMain(int argc, char *pArgv[])
{
	result r = E_SUCCESS;

	ArrayList* pArgs = new ArrayList();
	pArgs->Construct();
	for (int i = 0; i < argc; i++)
		pArgs->Add(*(new String(pArgv[i])));

	r = Osp::App::Application::Execute(SSHBada::CreateInstance, pArgs);
	if (IsFailed(r))
	{
		AppLogException("Application execution failed-[%s].", GetErrorMessage(r));
		r &= 0x0000FFFF;
	}

	pArgs->RemoveAll(true);
	delete pArgs;

	return static_cast<int>(r);
}
#ifdef __cplusplus
}
#endif // __cplusplus
