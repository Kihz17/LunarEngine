#include "IKeyFrameListener.h"

class KeyFrameListener : public IKeyFrameListener
{
public:
	KeyFrameListener() = default;
	virtual ~KeyFrameListener() = default;

	virtual void OnKeyFrame(const KeyFrame& frame) override;
};