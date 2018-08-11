#pragma once

#include <stdint.h>
#include <memory>
#include <random>

class RandGen
{
public:
	const static uint32_t maxRand = std::random_device::max();
	static RandGen& getInstance()
	{
		static RandGen instance;
		return instance;
	}
	uint32_t getInteger() noexcept
	{
		return (*dist)(rd);
	}

protected:
	~RandGen() = default;

private:
	RandGen(const RandGen&) = delete;
	void operator=(const RandGen&) = delete;

private:
	RandGen() noexcept
	{
		regen = std::make_shared<std::mt19937>(rd());
		dist = std::make_shared<std::uniform_int_distribution<uint32_t>>(0, maxRand);
	}
	std::random_device rd;
	std::shared_ptr<std::mt19937> regen;
	std::shared_ptr<std::uniform_int_distribution<uint32_t> > dist;
};

#define RAND_INT(x)  (RandGen::getInstance().getInteger() % x)