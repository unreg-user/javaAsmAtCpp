#pragma once

#define DU_UNION(sName, rvType) union Du##sName { \
sName v; \
rvType rv; \
explicit constexpr UNION_CONSTR_EXPLTYPE(Du##sName, sName, v);};

#define DU_UNION_VIA_AL_DT(sName) DU_UNION(sName, sName##DT);
