#include "mfpch.h"
#include "Primitive3D.h"


namespace Primitive
{
    /*BasicCube CreateBasicCube()
    {
        return {
            BasicVertex{{-0.5f, -0.5f, -0.5f},    {0.0f, 0.0f, -1.0f}    },
            BasicVertex{{ 0.5f, -0.5f, -0.5f},    {0.0f, 0.0f, -1.0f}    },
            BasicVertex{{ 0.5f, 0.5f, -0.5f },    {0.0f, 0.0f, -1.0f}    },
            BasicVertex{{ 0.5f, 0.5f, -0.5f },    {0.0f, 0.0f, -1.0f}    },
            BasicVertex{{-0.5f, 0.5f, -0.5f },    {0.0f, 0.0f, -1.0f}    },
            BasicVertex{{-0.5f, -0.5f, -0.5f},    {0.0f, 0.0f, -1.0f}    },

            BasicVertex{{-0.5f, -0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f}    },
            BasicVertex{{ 0.5f, -0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f}    },
            BasicVertex{{ 0.5f, 0.5f, 0.5f  },    { 0.0f, 0.0f, 1.0f}    },
            BasicVertex{{ 0.5f, 0.5f, 0.5f  },    { 0.0f, 0.0f, 1.0f}    },
            BasicVertex{{-0.5f, 0.5f, 0.5f  },    { 0.0f, 0.0f, 1.0f}    },
            BasicVertex{{-0.5f, -0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f}    },

            BasicVertex{{-0.5f, 0.5f, 0.5f   },    {-1.0f, 0.0f, 0.0f}   },
            BasicVertex{{-0.5f, 0.5f, -0.5f  },    {-1.0f, 0.0f, 0.0f}   },
            BasicVertex{{-0.5f, -0.5f, -0.5f },    {-1.0f, 0.0f, 0.0f}   },
            BasicVertex{{-0.5f, -0.5f, -0.5f },    {-1.0f, 0.0f, 0.0f}   },
            BasicVertex{{-0.5f, -0.5f, 0.5f  },    {-1.0f, 0.0f, 0.0f}   },
            BasicVertex{{-0.5f, 0.5f, 0.5f   },    {-1.0f, 0.0f, 0.0f}   },

            BasicVertex{ {0.5f, 0.5f, 0.5f   },  {1.0f, 0.0f, 0.0f}     },
            BasicVertex{ {0.5f, 0.5f, -0.5f  },  {1.0f, 0.0f, 0.0f}     },
            BasicVertex{ {0.5f, -0.5f, -0.5f },  {1.0f, 0.0f, 0.0f}     },
            BasicVertex{ {0.5f, -0.5f, -0.5f },  {1.0f, 0.0f, 0.0f}     },
            BasicVertex{ {0.5f, -0.5f, 0.5f  },  {1.0f, 0.0f, 0.0f}     },
            BasicVertex{ {0.5f, 0.5f, 0.5f   },  {1.0f, 0.0f, 0.0f}     },

            BasicVertex{{-0.5f, -0.5f, -0.5f},  { 0.0f, -1.0f, 0.0f}   },
            BasicVertex{{ 0.5f, -0.5f, -0.5f},  { 0.0f, -1.0f, 0.0f}   },
            BasicVertex{{ 0.5f, -0.5f, 0.5f },  { 0.0f, -1.0f, 0.0f}   },
            BasicVertex{{ 0.5f, -0.5f, 0.5f },  { 0.0f, -1.0f, 0.0f}   },
            BasicVertex{{-0.5f, -0.5f, 0.5f },  { 0.0f, -1.0f, 0.0f}   },
            BasicVertex{{-0.5f, -0.5f, -0.5f},  { 0.0f, -1.0f, 0.0f}   },

            BasicVertex{{-0.5f, 0.5f, -0.5f},   { 0.0f, 1.0f, 0.0f} },
            BasicVertex{{ 0.5f, 0.5f, -0.5f},   { 0.0f, 1.0f, 0.0f} },
            BasicVertex{{ 0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f} },
            BasicVertex{{ 0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f} },
            BasicVertex{{-0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f} },
            BasicVertex{{-0.5f, 0.5f, -0.5f},   { 0.0f, 1.0f, 0.0f} }
        };
    }

    TextureCube CreateTextureCube()
    {
        return {
            TextureVertex{{-0.5f, -0.5f, -0.5f},    {0.0f, 0.0f, -1.0f},   { 0.f, 0.f },    0   },
            TextureVertex{{ 0.5f, -0.5f, -0.5f},    {0.0f, 0.0f, -1.0f},   { 1.f, 0.f },    0   },
            TextureVertex{{ 0.5f, 0.5f, -0.5f },    {0.0f, 0.0f, -1.0f},   { 1.f, 1.f },    0   },
            TextureVertex{{ 0.5f, 0.5f, -0.5f },    {0.0f, 0.0f, -1.0f},   { 1.f, 1.f },    0   },
            TextureVertex{{-0.5f, 0.5f, -0.5f },    {0.0f, 0.0f, -1.0f},   { 0.f, 1.f },    0   },
            TextureVertex{{-0.5f, -0.5f, -0.5f},    {0.0f, 0.0f, -1.0f},   { 0.f, 0.f },    0   },
                                                                                            
            TextureVertex{{-0.5f, -0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f},   { 0.f, 0.f },    0      },
            TextureVertex{{ 0.5f, -0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f},   { 1.f, 0.f },    0      },
            TextureVertex{{ 0.5f, 0.5f, 0.5f  },    { 0.0f, 0.0f, 1.0f},   { 1.f, 1.f },    0      },
            TextureVertex{{ 0.5f, 0.5f, 0.5f  },    { 0.0f, 0.0f, 1.0f},   { 1.f, 1.f },    0      },
            TextureVertex{{-0.5f, 0.5f, 0.5f  },    { 0.0f, 0.0f, 1.0f},   { 0.f, 1.f },    0      },
            TextureVertex{{-0.5f, -0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f},   { 0.f, 0.f },    0      },
                                                                                            
            TextureVertex{{-0.5f, 0.5f, 0.5f   },   {-1.0f, 0.0f, 0.0f},   { 1.0f, 0.0f },    0      },
            TextureVertex{{-0.5f, 0.5f, -0.5f  },   {-1.0f, 0.0f, 0.0f},   { 1.0f, 1.0f },    0      },
            TextureVertex{{-0.5f, -0.5f, -0.5f },   {-1.0f, 0.0f, 0.0f},   { 0.0f, 1.0f },    0      },
            TextureVertex{{-0.5f, -0.5f, -0.5f },   {-1.0f, 0.0f, 0.0f},   { 0.0f, 1.0f },    0      },
            TextureVertex{{-0.5f, -0.5f, 0.5f  },   {-1.0f, 0.0f, 0.0f},   { 0.0f, 0.0f },    0      },
            TextureVertex{{-0.5f, 0.5f, 0.5f   },   {-1.0f, 0.0f, 0.0f},   { 1.0f, 0.0f },    0      },
                                                                                            
            TextureVertex{ {0.5f, 0.5f, 0.5f   },  { 1.0f, 0.0f, 0.0f },   { 1.f, 0.f },    0      },
            TextureVertex{ {0.5f, 0.5f, -0.5f  },  { 1.0f, 0.0f, 0.0f },   { 1.f, 1.f },    0      },
            TextureVertex{ {0.5f, -0.5f, -0.5f },  { 1.0f, 0.0f, 0.0f },   { 0.f, 1.f },    0      },
            TextureVertex{ {0.5f, -0.5f, -0.5f },  { 1.0f, 0.0f, 0.0f },   { 0.f, 1.f },    0      },
            TextureVertex{ {0.5f, -0.5f, 0.5f  },  { 1.0f, 0.0f, 0.0f },   { 0.f, 0.f },    0      },
            TextureVertex{ {0.5f, 0.5f, 0.5f   },  { 1.0f, 0.0f, 0.0f },   { 1.f, 0.f },    0      },
                                                                           
            TextureVertex{{-0.5f, -0.5f, -0.5f},   { 0.0f, -1.0f, 0.0f },   { 0.f, 1.f },   0     },
            TextureVertex{{ 0.5f, -0.5f, -0.5f},   { 0.0f, -1.0f, 0.0f },   { 1.f, 1.f },   0     },
            TextureVertex{{ 0.5f, -0.5f, 0.5f },   { 0.0f, -1.0f, 0.0f },   { 1.f, 0.f },   0     },
            TextureVertex{{ 0.5f, -0.5f, 0.5f },   { 0.0f, -1.0f, 0.0f },   { 1.f, 0.f },   0     },
            TextureVertex{{-0.5f, -0.5f, 0.5f },   { 0.0f, -1.0f, 0.0f },   { 0.f, 0.f },   0     },
            TextureVertex{{-0.5f, -0.5f, -0.5f},   { 0.0f, -1.0f, 0.0f },   { 0.f, 1.f },   0     },
            
            TextureVertex{{-0.5f, 0.5f, -0.5f},    { 0.0f, 1.0f, 0.0f},     { 0.f, 1.f},    0    },
            TextureVertex{{ 0.5f, 0.5f, -0.5f},    { 0.0f, 1.0f, 0.0f},     { 1.f, 1.f},    0    },
            TextureVertex{{ 0.5f, 0.5f, 0.5f },    { 0.0f, 1.0f, 0.0f},     { 1.f, 0.f},    0    },
            TextureVertex{{ 0.5f, 0.5f, 0.5f },    { 0.0f, 1.0f, 0.0f},     { 1.f, 0.f},    0    },
            TextureVertex{{-0.5f, 0.5f, 0.5f },    { 0.0f, 1.0f, 0.0f},     { 0.f, 0.f},    0    },
            TextureVertex{{-0.5f, 0.5f, -0.5f},    { 0.0f, 1.0f, 0.0f},     { 0.f, 1.f},    0    }
        };
    }*/
}