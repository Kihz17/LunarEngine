#pragma once

#include "GrassCluster.h"
#include "ISerializer.h"

class GrassSerializer : public ISerializer
{
public:
	GrassSerializer(GrassCluster& cluster);

	virtual void Serialize(YAML::Emitter& emitter) override;
	virtual void Deserialize(const YAML::Node& node) override;

private:
	GrassCluster& cluster;
};