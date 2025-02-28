/**
 * @file keywords.cpp
 * Manages the table of keywords.
 *
 * @author  Ben Gardner
 * @author  Guy Maurel since version 0.62 for uncrustify4Qt
 *          October 2015, 2016
 * @license GPL v2+
 */

#include "keywords.h"

#include "args.h"
#include "prototypes.h"
#include "uncrustify.h"
#include "uncrustify_limits.h"

#include <cerrno>
#include <map>


using namespace std;

// Dynamic keyword map
typedef map<string, E_Token> dkwmap;
static dkwmap dkwm;


/**
 * Compares two chunk_tag_t entries using strcmp on the strings
 *
 * @param the 'left' entry
 * @param the 'right' entry
 *
 * @return == 0  if both keywords are equal
 * @return  < 0  p1 is smaller than p2
 * @return  > 0  p2 is smaller than p1
 */
static int kw_compare(const void *p1, const void *p2);


/**
 * search in static keywords for first occurrence of a given tag
 *
 * @param tag/keyword to search for
 */
static const chunk_tag_t *kw_static_first(const chunk_tag_t *tag);


static const chunk_tag_t *kw_static_match(bool orig_list, const chunk_tag_t *tag, int lang_flags);

/**
 * selected keywords for the chosen language.
 */

static chunk_tag_t keyword_for_lang[uncrustify::limits::MAX_KEYWORDS];
static size_t      language_count;

/**
 * interesting static keywords - keep sorted.
 * Table includes the Name, Type, and Language flags.
 */
static chunk_tag_t keywords[] =
{
   // TODO: it might be useful if users could add their custom keywords to this list
   { "@autoreleasepool",                CT_AUTORELEASEPOOL,  LANG_OC                                                                     },
   { "@available",                      CT_OC_AVAILABLE,     LANG_OC                                                                     },
   { "@catch",                          CT_CATCH,            LANG_OC                                                                     },
   { "@dynamic",                        CT_OC_DYNAMIC,       LANG_OC                                                                     },
   { "@end",                            CT_OC_END,           LANG_OC                                                                     },
   { "@finally",                        CT_FINALLY,          LANG_OC                                                                     },
   { "@implementation",                 CT_OC_IMPL,          LANG_OC                                                                     },
   { "@interface",                      CT_OC_INTF,          LANG_OC                                                                     },
   { "@interface",                      CT_CLASS,            LANG_JAVA                                                                   },
   { "@private",                        CT_ACCESS,           LANG_OC                                                                     },
   { "@property",                       CT_OC_PROPERTY,      LANG_OC                                                                     },
   { "@protected",                      CT_ACCESS,           LANG_OC                                                                     },
   { "@protocol",                       CT_OC_PROTOCOL,      LANG_OC                                                                     },
   { "@public",                         CT_ACCESS,           LANG_OC                                                                     },
   { "@selector",                       CT_OC_SEL,           LANG_OC                                                                     },
   { "@synchronized",                   CT_SYNCHRONIZED,     LANG_OC                                                                     },
   { "@synthesize",                     CT_OC_DYNAMIC,       LANG_OC                                                                     },
   { "@throw",                          CT_THROW,            LANG_OC                                                                     },
   { "@try",                            CT_TRY,              LANG_OC                                                                     },
   { "API_AVAILABLE",                   CT_ATTRIBUTE,        LANG_OC                                                                     },
   { "API_DEPRECATED",                  CT_ATTRIBUTE,        LANG_OC                                                                     },
   { "API_DEPRECATED_WITH_REPLACEMENT", CT_ATTRIBUTE,        LANG_OC                                                                     },
   { "API_UNAVAILABLE",                 CT_ATTRIBUTE,        LANG_OC                                                                     },
   { "BOOL",                            CT_TYPE,             LANG_OC                                                                     },
   { "INT16_C",                         CT_TYPE,             LANG_CPP                                                                    },
   { "INT32_C",                         CT_TYPE,             LANG_CPP                                                                    },
   { "INT64_C",                         CT_TYPE,             LANG_CPP                                                                    },
   { "INT8_C",                          CT_TYPE,             LANG_CPP                                                                    },
   { "INTMAX_C",                        CT_TYPE,             LANG_CPP                                                                    },
   { "NS_ENUM",                         CT_ENUM,             LANG_OC                                                                     },
   { "NS_OPTIONS",                      CT_ENUM,             LANG_OC                                                                     },
   { "Q_EMIT",                          CT_Q_EMIT,           LANG_CPP                                                                    },
   { "Q_FOREACH",                       CT_FOR,              LANG_CPP                                                                    },
   { "Q_FOREVER",                       CT_Q_FOREVER,        LANG_CPP                                                                    },
   { "Q_GADGET",                        CT_Q_GADGET,         LANG_CPP                                                                    },
   { "Q_OBJECT",                        CT_COMMENT_EMBED,    LANG_CPP                                                                    },
   { "Q_SIGNALS",                       CT_ACCESS,           LANG_CPP                                                                    },
   { "UINT16_C",                        CT_TYPE,             LANG_CPP                                                                    },
   { "UINT32_C",                        CT_TYPE,             LANG_CPP                                                                    },
   { "UINT64_C",                        CT_TYPE,             LANG_CPP                                                                    },
   { "UINT8_C",                         CT_TYPE,             LANG_CPP                                                                    },
   { "UINTMAX_C",                       CT_TYPE,             LANG_CPP                                                                    },
   { "_Bool",                           CT_TYPE,             LANG_C | LANG_CPP                                                           },
   { "_Complex",                        CT_TYPE,             LANG_C | LANG_CPP                                                           },
   { "_Imaginary",                      CT_TYPE,             LANG_C | LANG_CPP                                                           },
   { "_Nonnull",                        CT_QUALIFIER,        LANG_OC                                                                     },
   { "_Null_unspecified",               CT_QUALIFIER,        LANG_OC                                                                     },
   { "_Nullable",                       CT_QUALIFIER,        LANG_OC                                                                     },
   { "_Pragma",                         CT_PP_PRAGMA,        LANG_ALL | FLAG_PP                                                          },
   { "__DI__",                          CT_DI,               LANG_C | LANG_CPP                                                           },
   { "__HI__",                          CT_HI,               LANG_C | LANG_CPP                                                           },
   { "__QI__",                          CT_QI,               LANG_C | LANG_CPP                                                           },
   { "__SI__",                          CT_SI,               LANG_C | LANG_CPP                                                           },
   { "__asm__",                         CT_ASM,              LANG_C | LANG_CPP                                                           },
   { "__attribute__",                   CT_ATTRIBUTE,        LANG_C | LANG_CPP | LANG_OC                                                 },
   { "__autoreleasing",                 CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__block",                         CT_QUALIFIER,        LANG_C | LANG_CPP | LANG_OC                                                 },
   { "__bridge",                        CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__bridge_retained",               CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__bridge_transfer",               CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__const__",                       CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__declspec",                      CT_DECLSPEC,         LANG_C | LANG_CPP                                                           },
   { "__except",                        CT_CATCH,            LANG_C | LANG_CPP                                                           },
   { "__finally",                       CT_FINALLY,          LANG_C | LANG_CPP                                                           },
   { "__has_include",                   CT_CNG_HASINC,       LANG_C | LANG_CPP | LANG_OC | FLAG_PP                                       },
   { "__has_include_next",              CT_CNG_HASINCN,      LANG_C | LANG_CPP | FLAG_PP                                                 },
   { "__inline__",                      CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__nonnull",                       CT_QUALIFIER,        LANG_OC                                                                     },
   { "__nothrow__",                     CT_NOTHROW,          LANG_C | LANG_CPP                                                           },
   { "__null_unspecified",              CT_QUALIFIER,        LANG_OC                                                                     },
   { "__nullable",                      CT_QUALIFIER,        LANG_OC                                                                     },
   { "__pragma",                        CT_PP_PRAGMA,        LANG_ALL | FLAG_PP                                                          },
   { "__restrict",                      CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__signed__",                      CT_TYPE,             LANG_C | LANG_CPP                                                           },
   { "__strong",                        CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__thread",                        CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__traits",                        CT_QUALIFIER,        LANG_D                                                                      },
   { "__try",                           CT_TRY,              LANG_C | LANG_CPP                                                           },
   { "__typeof",                        CT_DECLTYPE,         LANG_C | LANG_CPP | LANG_OC                                                 },
   { "__typeof__",                      CT_DECLTYPE,         LANG_C | LANG_CPP                                                           },
   { "__unsafe_unretained",             CT_QUALIFIER,        LANG_OC                                                                     },
   { "__unused",                        CT_ATTRIBUTE,        LANG_C | LANG_CPP                                                           },
   { "__volatile__",                    CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__weak",                          CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "__word__",                        CT_WORD_,            LANG_C | LANG_CPP                                                           },
   { "abstract",                        CT_QUALIFIER,        LANG_CS | LANG_D | LANG_JAVA | LANG_VALA | LANG_ECMA                        },
   { "add",                             CT_GETSET,           LANG_CS                                                                     },
   { "alias",                           CT_USING,            LANG_D                                                                      },
   { "align",                           CT_ALIGN,            LANG_D                                                                      },
   { "alignof",                         CT_SIZEOF,           LANG_CPP                                                                    },
   { "and",                             CT_SBOOL,            LANG_CPP                                                                    },
   { "and_eq",                          CT_SASSIGN,          LANG_CPP                                                                    },
   { "as",                              CT_AS,               LANG_CS | LANG_VALA                                                         },
   { "asm",                             CT_ASM,              LANG_C | LANG_CPP | LANG_D                                                  },
   { "asm",                             CT_PP_ASM,           LANG_ALL | FLAG_PP                                                          },
   { "assert",                          CT_ASSERT,           LANG_JAVA                                                                   },
   { "assert",                          CT_FUNCTION,         LANG_D | LANG_PAWN                                                          },
   { "assert",                          CT_PP_ASSERT,        LANG_PAWN | FLAG_PP                                                         },
   { "auto",                            CT_TYPE,             LANG_C | LANG_CPP | LANG_D                                                  },
   { "base",                            CT_BASE,             LANG_CS | LANG_VALA                                                         },
   { "bit",                             CT_TYPE,             LANG_D                                                                      },
   { "bitand",                          CT_ARITH,            LANG_C | LANG_CPP                                                           },
   { "bitor",                           CT_ARITH,            LANG_C | LANG_CPP                                                           },
   { "body",                            CT_BODY,             LANG_D                                                                      },
   { "bool",                            CT_TYPE,             LANG_C | LANG_CPP | LANG_CS | LANG_VALA                                     },
   { "boolean",                         CT_TYPE,             LANG_JAVA | LANG_ECMA                                                       },
   { "break",                           CT_BREAK,            LANG_ALL                                                                    },
   { "byte",                            CT_TYPE,             LANG_CS | LANG_D | LANG_JAVA | LANG_ECMA                                    },
   { "callback",                        CT_QUALIFIER,        LANG_VALA                                                                   },
   { "case",                            CT_CASE,             LANG_ALL                                                                    },
   { "cast",                            CT_D_CAST,           LANG_D                                                                      },
   { "catch",                           CT_CATCH,            LANG_CPP | LANG_CS | LANG_VALA | LANG_D | LANG_JAVA | LANG_ECMA             },
   { "cdouble",                         CT_TYPE,             LANG_D                                                                      },
   { "cent",                            CT_TYPE,             LANG_D                                                                      },
   { "cfloat",                          CT_TYPE,             LANG_D                                                                      },
   { "char",                            CT_CHAR,             LANG_PAWN                                                                   },
   { "char",                            CT_TYPE,             LANG_ALLC                                                                   },
   { "checked",                         CT_QUALIFIER,        LANG_CS                                                                     },
   { "class",                           CT_CLASS,            LANG_CPP | LANG_CS | LANG_D | LANG_JAVA | LANG_VALA | LANG_ECMA             },
   { "compl",                           CT_ARITH,            LANG_CPP                                                                    },
   { "const",                           CT_QUALIFIER,        LANG_ALL                                                                    },
   { "const_cast",                      CT_TYPE_CAST,        LANG_CPP                                                                    },
   { "constexpr",                       CT_QUALIFIER,        LANG_CPP                                                                    },
   { "construct",                       CT_CONSTRUCT,        LANG_VALA                                                                   },
   { "continue",                        CT_CONTINUE,         LANG_ALL                                                                    },
   { "creal",                           CT_TYPE,             LANG_D                                                                      },
   { "dchar",                           CT_TYPE,             LANG_D                                                                      },
   { "debug",                           CT_DEBUG,            LANG_D                                                                      },
   { "debugger",                        CT_DEBUGGER,         LANG_ECMA                                                                   },
   { "decltype",                        CT_DECLTYPE,         LANG_CPP                                                                    },
   { "default",                         CT_DEFAULT,          LANG_ALL                                                                    },
   { "define",                          CT_PP_DEFINE,        LANG_ALL | FLAG_PP                                                          },
   { "defined",                         CT_DEFINED,          LANG_PAWN                                                                   },
   { "defined",                         CT_PP_DEFINED,       LANG_ALLC | FLAG_PP                                                         },
   { "delegate",                        CT_DELEGATE,         LANG_CS | LANG_VALA | LANG_D                                                },
   { "delete",                          CT_DELETE,           LANG_CPP | LANG_D | LANG_ECMA | LANG_VALA                                   },
   { "deprecated",                      CT_QUALIFIER,        LANG_D                                                                      },
   { "do",                              CT_DO,               LANG_ALL                                                                    },
   { "double",                          CT_TYPE,             LANG_ALLC                                                                   },
   { "dynamic_cast",                    CT_TYPE_CAST,        LANG_CPP                                                                    },
   { "elif",                            CT_PP_ELSE,          LANG_ALLC | FLAG_PP                                                         },
   { "else",                            CT_ELSE,             LANG_ALL                                                                    },
   { "else",                            CT_PP_ELSE,          LANG_ALL | FLAG_PP                                                          },
   { "elseif",                          CT_PP_ELSE,          LANG_PAWN | FLAG_PP                                                         },
   { "emit",                            CT_PP_EMIT,          LANG_PAWN | FLAG_PP                                                         },
   { "endif",                           CT_PP_ENDIF,         LANG_ALL | FLAG_PP                                                          },
   { "endinput",                        CT_PP_ENDINPUT,      LANG_PAWN | FLAG_PP                                                         },
   { "endregion",                       CT_PP_ENDREGION,     LANG_ALL | FLAG_PP                                                          },
   { "endscript",                       CT_PP_ENDINPUT,      LANG_PAWN | FLAG_PP                                                         },
   { "enum",                            CT_ENUM,             LANG_ALL                                                                    },
   { "error",                           CT_PP_ERROR,         LANG_PAWN | FLAG_PP                                                         },
   { "errordomain",                     CT_ENUM,             LANG_VALA                                                                   },
   { "event",                           CT_TYPE,             LANG_CS                                                                     },
   { "exit",                            CT_FUNCTION,         LANG_PAWN                                                                   },
   { "explicit",                        CT_QUALIFIER,        LANG_CPP | LANG_CS                                                          },
   { "export",                          CT_EXPORT,           LANG_CPP | LANG_D | LANG_ECMA                                               },
   { "extends",                         CT_QUALIFIER,        LANG_JAVA | LANG_ECMA                                                       },
   { "extern",                          CT_EXTERN,           LANG_C | LANG_CPP | LANG_OC | LANG_CS | LANG_D | LANG_VALA                  },
   { "false",                           CT_WORD,             LANG_ALL                                                                    },
   { "file",                            CT_PP_FILE,          LANG_PAWN | FLAG_PP                                                         },
   { "final",                           CT_QUALIFIER,        LANG_CPP | LANG_D | LANG_ECMA                                               },
   { "finally",                         CT_FINALLY,          LANG_D | LANG_CS | LANG_VALA | LANG_ECMA | LANG_JAVA                        },
   { "fixed",                           CT_FIXED,            LANG_CS                                                                     },
   { "flags",                           CT_TYPE,             LANG_VALA                                                                   },
   { "float",                           CT_TYPE,             LANG_ALLC                                                                   },
   { "for",                             CT_FOR,              LANG_ALL                                                                    },
   { "foreach",                         CT_FOR,              LANG_CS | LANG_D | LANG_VALA                                                },
   { "foreach_reverse",                 CT_FOR,              LANG_D                                                                      },
   { "forward",                         CT_FORWARD,          LANG_PAWN                                                                   },
   { "friend",                          CT_FRIEND,           LANG_CPP                                                                    },
   { "function",                        CT_FUNCTION,         LANG_D | LANG_ECMA                                                          },
   { "get",                             CT_GETSET,           LANG_CS | LANG_VALA                                                         },
   { "goto",                            CT_GOTO,             LANG_ALL                                                                    },
   { "idouble",                         CT_TYPE,             LANG_D                                                                      },
   { "if",                              CT_IF,               LANG_ALL                                                                    },
   { "if",                              CT_PP_IF,            LANG_ALL | FLAG_PP                                                          },
   { "ifdef",                           CT_PP_IF,            LANG_ALLC | FLAG_PP                                                         },
   { "ifloat",                          CT_TYPE,             LANG_D                                                                      },
   { "ifndef",                          CT_PP_IF,            LANG_ALLC | FLAG_PP                                                         },
   { "implements",                      CT_QUALIFIER,        LANG_JAVA | LANG_ECMA                                                       },
   { "implicit",                        CT_QUALIFIER,        LANG_CS                                                                     },
   { "import",                          CT_IMPORT,           LANG_D | LANG_JAVA | LANG_ECMA                                              },
   { "import",                          CT_PP_INCLUDE,       LANG_OC | FLAG_PP                                                           },
   { "in",                              CT_IN,               LANG_D | LANG_CS | LANG_VALA | LANG_ECMA | LANG_OC                          },
   { "include",                         CT_PP_INCLUDE,       LANG_C | LANG_CPP | LANG_OC | LANG_PAWN | FLAG_PP                           },
   { "inline",                          CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "inout",                           CT_QUALIFIER,        LANG_D                                                                      },
   { "instanceof",                      CT_SIZEOF,           LANG_JAVA | LANG_ECMA                                                       },
   { "int",                             CT_TYPE,             LANG_ALLC                                                                   },
   { "interface",                       CT_CLASS,            LANG_CPP | LANG_CS | LANG_D | LANG_JAVA | LANG_VALA | LANG_ECMA             },
   { "internal",                        CT_QUALIFIER,        LANG_CS | LANG_VALA                                                         },
   { "invariant",                       CT_INVARIANT,        LANG_D                                                                      },
   { "ireal",                           CT_TYPE,             LANG_D                                                                      },
   { "is",                              CT_SCOMPARE,         LANG_D | LANG_CS | LANG_VALA                                                },
   { "lazy",                            CT_LAZY,             LANG_D                                                                      },
   { "line",                            CT_PP_LINE,          LANG_PAWN | FLAG_PP                                                         },
   { "lock",                            CT_LOCK,             LANG_CS | LANG_VALA                                                         },
   { "long",                            CT_TYPE,             LANG_ALLC                                                                   },
   { "macro",                           CT_D_MACRO,          LANG_D                                                                      },
   { "mixin",                           CT_CLASS,            LANG_D                                                                      }, // may need special handling
   { "module",                          CT_D_MODULE,         LANG_D                                                                      },
   { "mutable",                         CT_QUALIFIER,        LANG_CPP                                                                    },
   { "namespace",                       CT_NAMESPACE,        LANG_CPP | LANG_CS | LANG_VALA                                              },
   { "native",                          CT_NATIVE,           LANG_PAWN                                                                   },
   { "native",                          CT_QUALIFIER,        LANG_JAVA | LANG_ECMA                                                       },
   { "new",                             CT_NEW,              LANG_CPP | LANG_CS | LANG_D | LANG_JAVA | LANG_PAWN | LANG_VALA | LANG_ECMA },
   { "noexcept",                        CT_NOEXCEPT,         LANG_CPP                                                                    },
   { "nonnull",                         CT_TYPE,             LANG_OC                                                                     },
   { "not",                             CT_SARITH,           LANG_CPP                                                                    },
   { "not_eq",                          CT_SCOMPARE,         LANG_CPP                                                                    },
// { "null",                            CT_TYPE,             LANG_CS | LANG_D | LANG_JAVA | LANG_VALA                                    },
   { "null_resettable",                 CT_OC_PROPERTY_ATTR, LANG_OC                                                                     },
   { "null_unspecified",                CT_TYPE,             LANG_OC                                                                     },
   { "nullable",                        CT_TYPE,             LANG_OC                                                                     },
   { "object",                          CT_TYPE,             LANG_CS                                                                     },
   { "operator",                        CT_OPERATOR,         LANG_CPP | LANG_CS | LANG_PAWN                                              },
   { "or",                              CT_SBOOL,            LANG_CPP                                                                    },
   { "or_eq",                           CT_SASSIGN,          LANG_CPP                                                                    },
   { "out",                             CT_QUALIFIER,        LANG_CS | LANG_D | LANG_VALA                                                },
   { "override",                        CT_QUALIFIER,        LANG_CPP | LANG_CS | LANG_D | LANG_VALA                                     },
   { "package",                         CT_ACCESS,           LANG_D                                                                      },
   { "package",                         CT_PACKAGE,          LANG_ECMA | LANG_JAVA                                                       },
   { "params",                          CT_TYPE,             LANG_CS | LANG_VALA                                                         },
   { "pragma",                          CT_PP_PRAGMA,        LANG_ALL | FLAG_PP                                                          },
   { "private",                         CT_ACCESS,           LANG_ALLC                                                                   }, // not C
   { "property",                        CT_PP_PROPERTY,      LANG_CS | FLAG_PP                                                           },
   { "protected",                       CT_ACCESS,           LANG_ALLC                                                                   }, // not C
   { "public",                          CT_ACCESS,           LANG_ALL                                                                    }, // PAWN // not C
   { "readonly",                        CT_QUALIFIER,        LANG_CS                                                                     },
   { "real",                            CT_TYPE,             LANG_D                                                                      },
   { "ref",                             CT_QUALIFIER,        LANG_CS | LANG_VALA                                                         },
   { "region",                          CT_PP_REGION,        LANG_ALL | FLAG_PP                                                          },
   { "register",                        CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "reinterpret_cast",                CT_TYPE_CAST,        LANG_CPP                                                                    },
   { "remove",                          CT_GETSET,           LANG_CS                                                                     },
   { "restrict",                        CT_QUALIFIER,        LANG_C | LANG_CPP                                                           },
   { "return",                          CT_RETURN,           LANG_ALL                                                                    },
   { "sbyte",                           CT_TYPE,             LANG_CS                                                                     },
   { "scope",                           CT_D_SCOPE,          LANG_D                                                                      },
   { "sealed",                          CT_QUALIFIER,        LANG_CS                                                                     },
   { "section",                         CT_PP_SECTION,       LANG_PAWN | FLAG_PP                                                         },
   { "self",                            CT_THIS,             LANG_OC                                                                     },
   { "set",                             CT_GETSET,           LANG_CS | LANG_VALA                                                         },
   { "short",                           CT_TYPE,             LANG_ALLC                                                                   },
   { "signal",                          CT_ACCESS,           LANG_VALA                                                                   },
   { "signals",                         CT_ACCESS,           LANG_CPP                                                                    },
   { "signed",                          CT_TYPE,             LANG_C | LANG_CPP                                                           },
   { "size_t",                          CT_TYPE,             LANG_ALLC                                                                   },
   { "sizeof",                          CT_SIZEOF,           LANG_C | LANG_CPP | LANG_CS | LANG_VALA | LANG_PAWN                         },
   { "sleep",                           CT_SIZEOF,           LANG_PAWN                                                                   },
   { "stackalloc",                      CT_NEW,              LANG_CS                                                                     },
   { "state",                           CT_STATE,            LANG_PAWN                                                                   },
   { "static",                          CT_QUALIFIER,        LANG_ALL                                                                    },
   { "static_cast",                     CT_TYPE_CAST,        LANG_CPP                                                                    },
   { "stock",                           CT_STOCK,            LANG_PAWN                                                                   },
   { "strictfp",                        CT_QUALIFIER,        LANG_JAVA                                                                   },
   { "string",                          CT_TYPE,             LANG_CS | LANG_VALA                                                         },
   { "struct",                          CT_STRUCT,           LANG_C | LANG_CPP | LANG_OC | LANG_CS | LANG_D | LANG_VALA                  },
   { "super",                           CT_SUPER,            LANG_D | LANG_JAVA | LANG_ECMA                                              },
   { "switch",                          CT_SWITCH,           LANG_ALL                                                                    },
   { "synchronized",                    CT_QUALIFIER,        LANG_D | LANG_ECMA                                                          },
   { "synchronized",                    CT_SYNCHRONIZED,     LANG_JAVA                                                                   },
   { "tagof",                           CT_TAGOF,            LANG_PAWN                                                                   },
   { "template",                        CT_TEMPLATE,         LANG_CPP | LANG_D                                                           },
   { "this",                            CT_THIS,             LANG_CPP | LANG_CS | LANG_D | LANG_JAVA | LANG_VALA | LANG_ECMA             },
   { "throw",                           CT_THROW,            LANG_CPP | LANG_CS | LANG_VALA | LANG_D | LANG_JAVA | LANG_ECMA             },
   { "throws",                          CT_QUALIFIER,        LANG_JAVA | LANG_ECMA | LANG_VALA                                           },
   { "transient",                       CT_QUALIFIER,        LANG_JAVA | LANG_ECMA                                                       },
   { "true",                            CT_WORD,             LANG_ALL                                                                    },
   { "try",                             CT_TRY,              LANG_CPP | LANG_CS | LANG_D | LANG_JAVA | LANG_ECMA | LANG_VALA             },
   { "tryinclude",                      CT_PP_INCLUDE,       LANG_PAWN | FLAG_PP                                                         },
   { "typedef",                         CT_TYPEDEF,          LANG_C | LANG_CPP | LANG_OC | LANG_D                                        },
   { "typeid",                          CT_SIZEOF,           LANG_CPP | LANG_D                                                           },
   { "typename",                        CT_TYPENAME,         LANG_CPP                                                                    },
   { "typeof",                          CT_DECLTYPE,         LANG_C | LANG_CPP                                                           },
   { "typeof",                          CT_SIZEOF,           LANG_CS | LANG_D | LANG_VALA | LANG_ECMA                                    },
   { "ubyte",                           CT_TYPE,             LANG_D                                                                      },
   { "ucent",                           CT_TYPE,             LANG_D                                                                      },
   { "uint",                            CT_TYPE,             LANG_CS | LANG_VALA | LANG_D                                                },
   { "ulong",                           CT_TYPE,             LANG_CS | LANG_VALA | LANG_D                                                },
   { "unchecked",                       CT_QUALIFIER,        LANG_CS                                                                     },
   { "undef",                           CT_PP_UNDEF,         LANG_ALL | FLAG_PP                                                          },
   { "union",                           CT_UNION,            LANG_C | LANG_CPP | LANG_D                                                  },
   { "unittest",                        CT_UNITTEST,         LANG_D                                                                      },
   { "unsafe",                          CT_UNSAFE,           LANG_CS                                                                     },
   { "unsafe_unretained",               CT_QUALIFIER,        LANG_OC                                                                     },
   { "unsigned",                        CT_TYPE,             LANG_C | LANG_CPP                                                           },
   { "ushort",                          CT_TYPE,             LANG_CS | LANG_VALA | LANG_D                                                },
   { "using",                           CT_USING,            LANG_CPP | LANG_CS | LANG_VALA                                              },
   { "var",                             CT_TYPE,             LANG_CS | LANG_VALA | LANG_ECMA                                             },
   { "version",                         CT_D_VERSION,        LANG_D                                                                      },
   { "virtual",                         CT_QUALIFIER,        LANG_CPP | LANG_CS | LANG_VALA                                              },
   { "void",                            CT_TYPE,             LANG_ALLC                                                                   },
   { "volatile",                        CT_QUALIFIER,        LANG_C | LANG_CPP | LANG_CS | LANG_JAVA | LANG_ECMA                         },
   { "volatile",                        CT_VOLATILE,         LANG_D                                                                      },
   { "wchar",                           CT_TYPE,             LANG_D                                                                      },
   { "wchar_t",                         CT_TYPE,             LANG_C | LANG_CPP                                                           },
   { "weak",                            CT_QUALIFIER,        LANG_VALA                                                                   },
   { "when",                            CT_WHEN,             LANG_CS                                                                     },
   { "where",                           CT_WHERE,            LANG_CS                                                                     },
   { "while",                           CT_WHILE,            LANG_ALL                                                                    },
   { "with",                            CT_D_WITH,           LANG_D | LANG_ECMA                                                          },
   { "xor",                             CT_SARITH,           LANG_CPP                                                                    },
   { "xor_eq",                          CT_SASSIGN,          LANG_CPP                                                                    },
};


// Issue #3353
void init_keywords_for_language()
{
   unsigned int local_flags    = cpd.lang_flags;
   size_t       keywords_count = ARRAY_SIZE(keywords);

   language_count = 0;

   for (size_t idx = 0; idx < keywords_count; idx++)
   {
      chunk_tag_t *tag = &keywords[idx];

      if ((tag->lang_flags & local_flags) != 0)
      {
         // for debugging only
         // fprintf(stderr, "%s(%d): %zu Keyword: '%s', type is '%s'\n",
         //         __func__, __LINE__, idx, tag->tag, get_token_name(tag->type));
         keyword_for_lang[language_count].tag        = tag->tag;
         keyword_for_lang[language_count].type       = tag->type;
         keyword_for_lang[language_count].lang_flags = tag->lang_flags;
         language_count++;
      }
   }

   LOG_FMT(LDYNKW, "%s(%d): Number of Keywords for language %d: '%zu'\n",
           __func__, __LINE__, local_flags, language_count);
} // init_keywords_for_language


static int kw_compare(const void *p1, const void *p2)
{
   const chunk_tag_t *t1 = static_cast<const chunk_tag_t *>(p1);
   const chunk_tag_t *t2 = static_cast<const chunk_tag_t *>(p2);

   return(strcmp(t1->tag, t2->tag));
} // kw_compare


bool keywords_are_sorted()
{
   size_t keywords_count = ARRAY_SIZE(keywords);

   for (size_t idx = 1; idx < keywords_count; idx++)
   {
      if (kw_compare(&keywords[idx - 1], &keywords[idx]) > 0)
      {
         fprintf(stderr, "%s: bad sort order at idx %d, words '%s' and '%s'\n",
                 __func__, (int)idx - 1, keywords[idx - 1].tag, keywords[idx].tag);
         // coveralls will always complain.
         // these lines are only needed for the developer.
         log_flush(true);
         exit(EX_SOFTWARE);
      }
   }

   return(true);
} // keywords_are_sorted


void add_keyword(const std::string &tag, E_Token type)
{
   // See if the keyword has already been added
   dkwmap::iterator it = dkwm.find(tag);

   if (it != dkwm.end())
   {
      LOG_FMT(LDYNKW, "%s(%d): changed '%s' to '%s'\n",
              __func__, __LINE__, tag.c_str(), get_token_name(type));
      (*it).second = type;
      return;
   }
   // Insert the keyword
   dkwm.insert(dkwmap::value_type(tag, type));
   LOG_FMT(LDYNKW, "%s(%d): added '%s' as '%s'\n",
           __func__, __LINE__, tag.c_str(), get_token_name(type));
} // add_keyword


static const chunk_tag_t *kw_static_first(const chunk_tag_t *tag)
{
   const chunk_tag_t *prev = tag - 1;

   // TODO: avoid pointer arithmetic
   // loop over static keyword array
   while (  prev >= &keyword_for_lang[0]        // not at beginning of keyword array
         && strcmp(prev->tag, tag->tag) == 0)   // tags match
   {
      tag = prev;
      prev--;
   }
   return(tag);
} // kw_static_first


static const chunk_tag_t *kw_static_match(bool orig_list, const chunk_tag_t *tag, int lang_flags)
{
   bool in_pp = (  cpd.in_preproc != CT_NONE
                && cpd.in_preproc != CT_PP_DEFINE);

   for (const chunk_tag_t *iter = kw_static_first(tag);
        (orig_list) ? (iter < &keywords[ARRAY_SIZE(keywords)]) : (iter < &keyword_for_lang[language_count]);
        iter++)
   {
      bool pp_iter = (iter->lang_flags & FLAG_PP) != 0; // forcing value to bool

      if (  (strcmp(iter->tag, tag->tag) == 0)
         && language_is_set(iter->lang_flags)
         && (lang_flags & iter->lang_flags)
         && in_pp == pp_iter)
      {
         return(iter);
      }
   }

   return(nullptr);
} // kw_static_match


E_Token find_keyword_type(const char *word, size_t len)
{
   if (len <= 0)
   {
      return(CT_NONE);
   }
   // check the dynamic word list first
   string           ss(word, len);
   dkwmap::iterator it = dkwm.find(ss);

   if (it != dkwm.end())
   {
      return((*it).second);
   }
   chunk_tag_t key;

   key.tag = ss.c_str();

   // check the static word list
   const chunk_tag_t *p_ret = static_cast<const chunk_tag_t *>(
      bsearch(&key, keyword_for_lang, language_count, sizeof(keyword_for_lang[0]), kw_compare));

   if (p_ret != nullptr)
   {
      if (  strcmp(p_ret->tag, "__pragma") == 0
         || strcmp(p_ret->tag, "_Pragma") == 0)
      {
         cpd.in_preproc = CT_PREPROC;
      }
      p_ret = kw_static_match(false, p_ret, cpd.lang_flags);
   }
   return((p_ret != nullptr) ? p_ret->type : CT_WORD);
} // find_keyword_type


int load_keyword_file(const char *filename)
{
   FILE *pf = fopen(filename, "r");

   if (pf == nullptr)
   {
      LOG_FMT(LERR, "%s: fopen(%s) failed: %s (%d)\n",
              __func__, filename, strerror(errno), errno);
      exit(EX_IOERR);
   }
   const int max_line_length = 256;
   const int max_arg_count   = 2;

   // maximal length of a line in the file
   char   buf[max_line_length];
   char   *args[max_arg_count];
   size_t line_no = 0;

   // read file line by line
   while (fgets(buf, max_line_length, pf) != nullptr)
   {
      line_no++;

      // remove comments after '#' sign
      char *ptr;

      if ((ptr = strchr(buf, '#')) != nullptr)
      {
         *ptr = 0; // set string end where comment begins
      }
      size_t argc = Args::SplitLine(buf, args, max_arg_count);

      if (argc > 0)
      {
         if (  argc == 1
            && CharTable::IsKw1(*args[0]))
         {
            add_keyword(args[0], CT_TYPE);
         }
         else
         {
            LOG_FMT(LWARN, "%s:%zu Invalid line (starts with '%s')\n",
                    filename, line_no, args[0]);
            exit(EX_SOFTWARE);
         }
      }
      else
      {
         continue; // the line is empty
      }
   }
   fclose(pf);
   return(EX_OK);
} // load_keyword_file


void print_custom_keywords(FILE *pfile)
{
   for (const auto &keyword_pair : dkwm)
   {
      E_Token tt = keyword_pair.second;

      if (tt == CT_TYPE)
      {
         fprintf(pfile, "custom type %*.s%s\n",
                 uncrustify::limits::MAX_OPTION_NAME_LEN - 10, " ",
                 keyword_pair.first.c_str());
      }
      else if (tt == CT_MACRO_OPEN)
      {
         fprintf(pfile, "macro-open %*.s%s\n",
                 uncrustify::limits::MAX_OPTION_NAME_LEN - 11, " ",
                 keyword_pair.first.c_str());
      }
      else if (tt == CT_MACRO_CLOSE)
      {
         fprintf(pfile, "macro-close %*.s%s\n",
                 uncrustify::limits::MAX_OPTION_NAME_LEN - 12, " ",
                 keyword_pair.first.c_str());
      }
      else if (tt == CT_MACRO_ELSE)
      {
         fprintf(pfile, "macro-else %*.s%s\n",
                 uncrustify::limits::MAX_OPTION_NAME_LEN - 11, " ",
                 keyword_pair.first.c_str());
      }
      else
      {
         const char *tn = get_token_name(tt);

         fprintf(pfile, "set %s %*.s%s\n",
                 tn,
                 uncrustify::limits::MAX_OPTION_NAME_LEN - (4 + static_cast<int>(strlen(tn))),
                 " ", keyword_pair.first.c_str());
      }
   }
} // print_custom_keywords


void clear_keyword_file()
{
   dkwm.clear();
} // clear_keyword_file


pattern_class_e get_token_pattern_class(E_Token tok)
{
   // TODO: instead of this switch better assign the pattern class to each statement
   switch (tok)
   {
   case CT_IF:
   case CT_ELSEIF:
   case CT_SWITCH:
   case CT_FOR:
   case CT_WHILE:
   case CT_SYNCHRONIZED:
   case CT_USING_STMT:
   case CT_LOCK:
   case CT_D_WITH:
   case CT_D_VERSION_IF:
   case CT_D_SCOPE_IF:
      return(pattern_class_e::PBRACED);

   case CT_ELSE:
      return(pattern_class_e::ELSE);

   case CT_DO:
   case CT_TRY:
   case CT_FINALLY:
   case CT_BODY:
   case CT_UNITTEST:
   case CT_UNSAFE:
   case CT_VOLATILE:
   case CT_GETSET:
      return(pattern_class_e::BRACED);

   case CT_CATCH:
   case CT_D_VERSION:
   case CT_DEBUG:
      return(pattern_class_e::OPBRACED);

   case CT_NAMESPACE:
      return(pattern_class_e::VBRACED);

   case CT_WHILE_OF_DO:
      return(pattern_class_e::PAREN);

   case CT_INVARIANT:
      return(pattern_class_e::OPPAREN);

   default:
      return(pattern_class_e::NONE);
   } // switch
}    // get_token_pattern_class
