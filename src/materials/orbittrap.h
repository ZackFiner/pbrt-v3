#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_MATERIALS_ORBITTRAP_H
#define PBRT_MATERIALS_ORBITTRAP_H

// materials/matte.h*
#include "pbrt.h"
#include "material.h"

namespace pbrt {

	// MatteMaterial Declarations
	class OrbitTrapMaterial : public Material {
	public:
		OrbitTrapMaterial(const std::shared_ptr<Texture<Spectrum>> &Kd,
			const std::shared_ptr<Texture<Spectrum>> &Ks,
			const std::shared_ptr<Texture<Spectrum>> &Kr,
			const std::shared_ptr<Texture<Spectrum>> &Kt,
			const std::shared_ptr<Texture<Float>> &roughness,
			const std::shared_ptr<Texture<Float>> &roughnessu,
			const std::shared_ptr<Texture<Float>> &roughnessv,
			const std::shared_ptr<Texture<Spectrum>> &opacity,
			const std::shared_ptr<Texture<Float>> &eta,
			const std::shared_ptr<Texture<Float>> &bumpMap,
			bool remapRoughness)
			: Kd(Kd),
			Ks(Ks),
			Kr(Kr),
			Kt(Kt),
			opacity(opacity),
			roughness(roughness),
			roughnessu(roughnessu),
			roughnessv(roughnessv),
			eta(eta),
			bumpMap(bumpMap),
			remapRoughness(remapRoughness) {}

		void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
			TransportMode mode,
			bool allowMultipleLobes) const;

	private:
		// UberMaterial Private Data
		std::shared_ptr<Texture<Spectrum>> Kd, Ks, Kr, Kt, opacity;
		std::shared_ptr<Texture<Float>> roughness, roughnessu, roughnessv, eta,
			bumpMap;
		bool remapRoughness;
	};

	OrbitTrapMaterial *CreateOrbitTrapMaterial(const TextureParams &mp);

}  // namespace pbrt

#endif  // PBRT_MATERIALS_ORBITTRAP_H
