#include "materials/orbittrap.h"
#include "paramset.h"
#include "reflection.h"
#include "interaction.h"
#include "texture.h"
#include "interaction.h"

namespace pbrt {

	// MatteMaterial Method Definitions
	void OrbitTrapMaterial::ComputeScatteringFunctions(SurfaceInteraction *si,
		MemoryArena &arena,
		TransportMode mode,
		bool allowMultipleLobes) const {
		// Perform bump mapping with _bumpMap_, if present
		if (bumpMap) Bump(bumpMap, si);
		Float e = eta->Evaluate(*si);

		Spectrum op = opacity->Evaluate(*si).Clamp();
		Spectrum t = (-op + Spectrum(1.f)).Clamp();
		if (!t.IsBlack()) {
			si->bsdf = ARENA_ALLOC(arena, BSDF)(*si, 1.f);
			BxDF *tr = ARENA_ALLOC(arena, SpecularTransmission)(t, 1.f, 1.f, mode);
			si->bsdf->Add(tr);
		}
		else
			si->bsdf = ARENA_ALLOC(arena, BSDF)(*si, e);

		/*
		Spectrum col0 = Spectrum();
		col0[0] = 0.1f; col0[1] = 0.9f; col0[2] = 0.1f;
		Spectrum col1 = Spectrum();
		col1[0] = 0.9f; col1[1] = 0.1f; col1[2] = 0.1f;
		Spectrum col2 = Spectrum();
		col2[0] = 0.8f; col2[1] = 0.0f; col2[2] = 0.2f;
		Spectrum col3 = Spectrum();
		col3[0] = 0.1f; col3[1] = 0.1f; col3[2] = 0.9f;
		//Spectrum kd = op * Kd->Evaluate(*si).Clamp();
		Float index = acos(Dot(si->orbitTrap, si->n)) * InvPi;
		
		if (isNaN<Float>(index)) // NAN CHECK
			index = 1.0f;
		
		//std::cout << si->orbitTrap << std::endl;
		//std::cout << index << std::endl;
		Float p0 = 0.5f, pi0 = 1.0f / p0;
		Float p1 = 0.6f, pi1 = 1.0f / (p1-p0);
		Float p2 = 1.0f, pi2 = 1.0f / (p2-p1);
		//Float index = p2;
		Spectrum kd =  col0 + (col1 - col0)*Clamp(index*pi0, 0.0f, 1.0f)
							+ (col2 - col1)*Clamp((index - p0)*pi1, 0.0f, 1.0f)
							+ (col3 - col2)*Clamp((index - p1)*pi2, 0.0f, 1.0f);
		*/
		Spectrum kd = Spectrum();
		Float invDivRad = fudgeFactor;
		kd[0] = Clamp(si->orbitTrap.x * invDivRad, 0.0f, 1.0f);
		kd[1] = Clamp(si->orbitTrap.y * invDivRad, 0.0f, 1.0f);
		kd[2] = Clamp(si->orbitTrap.z * invDivRad, 0.0f, 1.0f);
		kd *= op;

		if (enableFakeAO)
			kd *= 1.0f - (si->rayMarchSteps / 1000.0f); // fake ambient occlusion from ray march steps

		if (!kd.IsBlack()) {
			BxDF *diff = ARENA_ALLOC(arena, LambertianReflection)(kd);
			si->bsdf->Add(diff);
		}
		
		Spectrum ks = op * Ks->Evaluate(*si).Clamp();
		
		if (!ks.IsBlack()) {
			Fresnel *fresnel = ARENA_ALLOC(arena, FresnelDielectric)(1.f, e);
			Float roughu, roughv;
			if (roughnessu)
				roughu = roughnessu->Evaluate(*si);
			else
				roughu = roughness->Evaluate(*si);
			if (roughnessv)
				roughv = roughnessv->Evaluate(*si);
			else
				roughv = roughu;
			if (remapRoughness) {
				roughu = TrowbridgeReitzDistribution::RoughnessToAlpha(roughu);
				roughv = TrowbridgeReitzDistribution::RoughnessToAlpha(roughv);
			}
			MicrofacetDistribution *distrib =
				ARENA_ALLOC(arena, TrowbridgeReitzDistribution)(roughu, roughv);
			BxDF *spec =
				ARENA_ALLOC(arena, MicrofacetReflection)(ks, distrib, fresnel);
			si->bsdf->Add(spec);
		}

		Spectrum kr = op * Kr->Evaluate(*si).Clamp();
		if (!kr.IsBlack()) {
			Fresnel *fresnel = ARENA_ALLOC(arena, FresnelDielectric)(1.f, e);
			si->bsdf->Add(ARENA_ALLOC(arena, SpecularReflection)(kr, fresnel));
		}

		Spectrum kt = op * Kt->Evaluate(*si).Clamp();
		if (!kt.IsBlack())
			si->bsdf->Add(
				ARENA_ALLOC(arena, SpecularTransmission)(kt, 1.f, e, mode));
	}

	OrbitTrapMaterial *CreateOrbitTrapMaterial(const TextureParams &mp) {
		std::shared_ptr<Texture<Spectrum>> Kd =
			mp.GetSpectrumTexture("Kd", Spectrum(0.25f));
		std::shared_ptr<Texture<Spectrum>> Ks =
			mp.GetSpectrumTexture("Ks", Spectrum(0.25f));
		std::shared_ptr<Texture<Spectrum>> Kr =
			mp.GetSpectrumTexture("Kr", Spectrum(0.f));
		std::shared_ptr<Texture<Spectrum>> Kt =
			mp.GetSpectrumTexture("Kt", Spectrum(0.f));



		std::shared_ptr<Texture<Float>> roughness =
			mp.GetFloatTexture("roughness", .1f);
		std::shared_ptr<Texture<Float>> uroughness =
			mp.GetFloatTextureOrNull("uroughness");
		std::shared_ptr<Texture<Float>> vroughness =
			mp.GetFloatTextureOrNull("vroughness");
		std::shared_ptr<Texture<Float>> eta = mp.GetFloatTextureOrNull("eta");
		if (!eta) eta = mp.GetFloatTexture("index", 1.5f);
		std::shared_ptr<Texture<Spectrum>> opacity =
			mp.GetSpectrumTexture("opacity", 1.f);
		std::shared_ptr<Texture<Float>> bumpMap =
			mp.GetFloatTextureOrNull("bumpmap");
		bool remapRoughness = mp.FindBool("remaproughness", true);
		bool enableFakeAO = mp.FindBool("enableFakeAO", false);
		Float fudgeFactor = mp.FindFloat("fudgeFactor", 1.0f);
		return new OrbitTrapMaterial(Kd, Ks, Kr, Kt, roughness, uroughness, vroughness,
			opacity, eta, bumpMap, remapRoughness, enableFakeAO, fudgeFactor);
	}

}  // namespace pbrt
