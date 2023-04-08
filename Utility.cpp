#include "Utility.h"

void NameResource(ID3D11DeviceChild* pDeviceChild, const std::string& name)
{
#if _DEBUG
    ThrowIfFailed(pDeviceChild->SetPrivateData(WKPDID_D3DDebugObjectName,
                                               UINT(name.length()),
                                               name.data()));
#endif // _DEBUG
}