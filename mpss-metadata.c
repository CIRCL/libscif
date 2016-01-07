#ifdef __GNUC__
#define SPECIFIERS static const __attribute__ ((used, section(".mpss-metadata")))
#else
#pragma section(".mpss-md", read)
#define SPECIFIERS static __declspec(allocate(".mpss-md"))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* include the metadata in the program binary */
SPECIFIERS char ____mpss_metadata[] =
	"MPSS_METADATA:\n"
	"commit\t" MPSS_COMMIT "\n"
	"version\t" MPSS_VERSION "\n"
	"buildno\t" MPSS_BUILDNO "\n"
	"builton\t" MPSS_BUILTON "\n"
	"builtby\t" MPSS_BUILTBY "\n"
	;

#ifdef __cplusplus
}
#endif
