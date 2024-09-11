#include "precomp.h"
#include "smoke.h"

namespace Tmpl8
{

void Smoke::tick()
{
    if (++current_frame == 60) current_frame = 0;
}

void Smoke::draw(Surface* screen)
{
    smoke_sprite.set_frame(current_frame / 15);

    smoke_sprite.draw(screen, (int)position.x + HEALTHBAR_OFFSET, (int)position.y);
}

} // namespace Tmpl8