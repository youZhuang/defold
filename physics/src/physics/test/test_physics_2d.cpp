/**
 * NOTE! This test is only present since the 3D physics does not yet support multiple
 * collision groups within the same collision object nor support for the custom grid-shape.
 */

#include "test_physics.h"

#include <dlib/math.h>

using namespace Vectormath::Aos;

VisualObject::VisualObject()
: m_Position(0.0f, 0.0f, 0.0f)
, m_Rotation(0.0f, 0.0f, 0.0f, 1.0f)
, m_CollisionCount(0)
, m_FirstCollisionGroup(0)
{

}

void GetWorldTransform(void* visual_object, Vectormath::Aos::Point3& position, Vectormath::Aos::Quat& rotation)
{
    if (visual_object != 0x0)
    {
        VisualObject* o = (VisualObject*) visual_object;
        position = o->m_Position;
        rotation = o->m_Rotation;
    }
    else
    {
        position = Vectormath::Aos::Point3(0.0f, 0.0f, 0.0f);
        rotation = Vectormath::Aos::Quat(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

void SetWorldTransform(void* visual_object, const Vectormath::Aos::Point3& position, const Vectormath::Aos::Quat& rotation)
{
    if (!visual_object) return;
    VisualObject* o = (VisualObject*) visual_object;
    o->m_Position = position;
    o->m_Rotation = rotation;
}

bool CollisionCallback(void* user_data_a, uint16_t group_a, void* user_data_b, uint16_t group_b, void* user_data)
{
    VisualObject* vo = (VisualObject*)user_data_a;
    if (vo->m_CollisionCount == 0)
        vo->m_FirstCollisionGroup = group_a;
    ++vo->m_CollisionCount;

    vo = (VisualObject*)user_data_b;
    if (vo->m_CollisionCount == 0)
        vo->m_FirstCollisionGroup = group_b;
    ++vo->m_CollisionCount;
    int* count = (int*)user_data;
    if (*count < 20)
    {
        *count += 1;
        return true;
    }
    else
    {
        return false;
    }
}

bool ContactPointCallback(const dmPhysics::ContactPoint& contact_point, void* user_data)
{
    (void) contact_point;
    (void) user_data;
    return true;
}

Test2D::Test2D()
: m_NewContextFunc(dmPhysics::NewContext2D)
, m_DeleteContextFunc(dmPhysics::DeleteContext2D)
, m_NewWorldFunc(dmPhysics::NewWorld2D)
, m_DeleteWorldFunc(dmPhysics::DeleteWorld2D)
, m_StepWorldFunc(dmPhysics::StepWorld2D)
, m_DrawDebugFunc(dmPhysics::DrawDebug2D)
, m_NewBoxShapeFunc(dmPhysics::NewBoxShape2D)
, m_NewSphereShapeFunc(dmPhysics::NewCircleShape2D)
, m_NewCapsuleShapeFunc(0x0)
, m_NewConvexHullShapeFunc(dmPhysics::NewPolygonShape2D)
, m_DeleteCollisionShapeFunc(dmPhysics::DeleteCollisionShape2D)
, m_NewCollisionObjectFunc(dmPhysics::NewCollisionObject2D)
, m_DeleteCollisionObjectFunc(dmPhysics::DeleteCollisionObject2D)
, m_GetCollisionShapesFunc(dmPhysics::GetCollisionShapes2D)
, m_SetCollisionObjectUserDataFunc(dmPhysics::SetCollisionObjectUserData2D)
, m_GetCollisionObjectUserDataFunc(dmPhysics::GetCollisionObjectUserData2D)
, m_ApplyForceFunc(dmPhysics::ApplyForce2D)
, m_GetTotalForceFunc(dmPhysics::GetTotalForce2D)
, m_GetWorldPositionFunc(dmPhysics::GetWorldPosition2D)
, m_GetWorldRotationFunc(dmPhysics::GetWorldRotation2D)
, m_GetLinearVelocityFunc(dmPhysics::GetLinearVelocity2D)
, m_GetAngularVelocityFunc(dmPhysics::GetAngularVelocity2D)
, m_RequestRayCastFunc(dmPhysics::RequestRayCast2D)
, m_SetDebugCallbacksFunc(dmPhysics::SetDebugCallbacks2D)
, m_ReplaceShapeFunc(dmPhysics::ReplaceShape2D)
, m_Vertices(new float[3*2])
, m_VertexCount(3)
, m_PolygonRadius(b2_polygonRadius)
{
    m_Vertices[0] = 0.0f; m_Vertices[1] = 0.0f;
    m_Vertices[2] = 1.0f; m_Vertices[3] = 0.0f;
    m_Vertices[4] = 0.0f; m_Vertices[5] = 1.0f;
}

Test2D::~Test2D()
{
    delete [] m_Vertices;
}

typedef ::testing::Types<Test2D> TestTypes;
TYPED_TEST_CASE(PhysicsTest, TestTypes);

TYPED_TEST(PhysicsTest, MultipleGroups)
{
    VisualObject vo_a;
    dmPhysics::CollisionObjectData data;
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_STATIC;
    data.m_Mass = 0.0f;
    data.m_UserData = &vo_a;
    data.m_Group = 1 << 2;
    data.m_Mask = 1 << 3;
    typename TypeParam::CollisionShapeType shapes[2];
    float v1[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    shapes[0] = (*TestFixture::m_Test.m_NewConvexHullShapeFunc)(TestFixture::m_Context, v1, 4);
    float v2[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 0.0f, 2.0f};
    shapes[1] = (*TestFixture::m_Test.m_NewConvexHullShapeFunc)(TestFixture::m_Context, v2, 4);
    typename TypeParam::CollisionObjectType static_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, shapes, 2u);
    dmPhysics::SetCollisionObjectFilter(static_co, 1, 0, 1 << 1, 1 << 3);

    VisualObject vo_b;
    vo_b.m_Position = Point3(0.5f, 3.0f, 0.0f);
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_DYNAMIC;
    data.m_Mass = 1.0f;
    data.m_UserData = &vo_b;
    data.m_Group = 1 << 3;
    data.m_Mask = 1 << 2;
    typename TypeParam::CollisionShapeType shape = (*TestFixture::m_Test.m_NewBoxShapeFunc)(TestFixture::m_Context, Vector3(0.5f, 0.5f, 0.0f));
    typename TypeParam::CollisionObjectType dynamic_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &shape, 1u);

    for (uint32_t i = 0; i < 40; ++i)
    {
        (*TestFixture::m_Test.m_StepWorldFunc)(TestFixture::m_World, TestFixture::m_StepWorldContext);
    }

    ASSERT_GT(1.5f + 2.0f * TestFixture::m_Test.m_PolygonRadius / PHYSICS_SCALE, vo_b.m_Position.getY());

    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, static_co);
    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, dynamic_co);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(shape);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(shapes[0]);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(shapes[1]);
}

TYPED_TEST(PhysicsTest, GridShapePolygon)
{
    int32_t rows = 4;
    int32_t columns = 10;
    int32_t cell_width = 16;
    int32_t cell_height = 16;
    float grid_radius = b2_polygonRadius;

    for (int32_t j = 0; j < columns; ++j)
    {
        VisualObject vo_a;
        vo_a.m_Position = Point3(0, 0, 0);
        dmPhysics::CollisionObjectData data;
        data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_STATIC;
        data.m_Mass = 0.0f;
        data.m_UserData = &vo_a;
        data.m_Group = 0xffff;
        data.m_Mask = 0xffff;
        data.m_Restitution = 0.0f;

        const float hull_vertices[] = {  // 1x1 around origo
                                        -0.5f, -0.5f,
                                         0.5f, -0.5f,
                                         0.5f,  0.5f,
                                        -0.5f,  0.5f,

                                         // 1x0.5 with top aligned to x-axis
                                        -0.5f, -0.5f,
                                         0.5f, -0.5f,
                                         0.5f,  0.0f,
                                        -0.5f,  0.0f };

        const dmPhysics::HullDesc hulls[] = { {0, 4}, {4, 4} };
        dmPhysics::HHullSet2D hull_set = dmPhysics::NewHullSet2D(TestFixture::m_Context, hull_vertices, 8, hulls, 2);
        dmPhysics::HCollisionShape2D grid_shape = dmPhysics::NewGridShape2D(TestFixture::m_Context, hull_set, Point3(0,0,0), cell_width, cell_height, rows, columns);
        typename TypeParam::CollisionObjectType grid_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &grid_shape, 1u);

        for (int32_t row = 0; row < rows; ++row)
        {
            for (int32_t col = 0; col < columns; ++col)
            {
                dmPhysics::SetGridShapeHull(grid_co, grid_shape, row, col, 0);
            }
        }

        VisualObject vo_b;
        vo_b.m_Position = Point3(-columns * cell_width * 0.5f + cell_width * 0.5f + cell_width * j, 50.0f, 0.0f);
        data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_DYNAMIC;
        data.m_Mass = 1.0f;
        data.m_UserData = &vo_b;
        data.m_Group = 0xffff;
        data.m_Mask = 0xffff;
        data.m_Restitution = 0.0f;
        typename TypeParam::CollisionShapeType shape = (*TestFixture::m_Test.m_NewBoxShapeFunc)(TestFixture::m_Context, Vector3(0.5f, 0.5f, 0.0f));
        typename TypeParam::CollisionObjectType dynamic_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &shape, 1u);

        for (int32_t col = 0; col < columns; ++col)
        {
            uint32_t child = columns * (rows - 1) + col;
            // Set group for last row to [0, 1, 2, 4, ...]. The box should not collide with the first object in the last row.
            // See ASSERT_NEAR below for the special case
            dmPhysics::SetCollisionObjectFilter(grid_co, 0, child, (1 << col) >> 1, 0xffff);
        }

        // Set the last cell in the last row to the smaller hull
        dmPhysics::SetGridShapeHull(grid_co, grid_shape, rows - 1, columns - 1, 1);

        // Remove the cell just before the last cell
        dmPhysics::SetGridShapeHull(grid_co, grid_shape, rows - 1, columns - 2, dmPhysics::GRIDSHAPE_EMPTY_CELL);

        // Remove the cell temporarily and add after first simulation.
        // Test this to make sure that the broad-phase is properly updated
        dmPhysics::SetGridShapeHull(grid_co, grid_shape, rows - 1, 2, dmPhysics::GRIDSHAPE_EMPTY_CELL);

        for (uint32_t i = 0; i < 400; ++i)
        {
            (*TestFixture::m_Test.m_StepWorldFunc)(TestFixture::m_World, TestFixture::m_StepWorldContext);

            // Set cell to hull 1 removed above. See comment above
            if (i == 0)
            {
                dmPhysics::SetGridShapeHull(grid_co, grid_shape, rows - 1, 2, 0);
            }
        }

        if (j == 0)
        {
            // Box should fall through last row to next
            ASSERT_NEAR(16.0f + grid_radius + 0.5f, vo_b.m_Position.getY(), 0.05f);
            ASSERT_EQ(0xffff, vo_a.m_FirstCollisionGroup);
        }
        else
        {
            // Should collide with last row

            if (j == columns - 1)
            {
                // Special case for last cell, the smaller hull
                ASSERT_NEAR(24.0f + grid_radius + 0.5f, vo_b.m_Position.getY(), 0.05f);
            }
            else if (j == columns - 2)
            {
                // Special case for the cell just before the last cell. The cell is removed
                // and the box should fall-through
                ASSERT_NEAR(16.0f + grid_radius + 0.5f, vo_b.m_Position.getY(), 0.05f);
            }
            else
            {
                // "General" case
                ASSERT_NEAR(32.0f + grid_radius + 0.5f, vo_b.m_Position.getY(), 0.05f);
            }

            if (j == columns - 2)
            {
                // Fall through case (removed hull), group should be 0xffff i
                ASSERT_EQ(0xffff, vo_a.m_FirstCollisionGroup);
            }
            else
            {
                ASSERT_EQ((1 << j) >> 1, vo_a.m_FirstCollisionGroup);
            }
        }

        (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, grid_co);
        (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, dynamic_co);
        (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(shape);
        (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(grid_shape);
        dmPhysics::DeleteHullSet2D(hull_set);
    }
}

TYPED_TEST(PhysicsTest, GridShapeSphere)
{
    /*
     * Simplified version of GridShapePolygon
     */
    int32_t rows = 4;
    int32_t columns = 10;
    int32_t cell_width = 16;
    int32_t cell_height = 16;

    VisualObject vo_a;
    vo_a.m_Position = Point3(1, 0, 0);
    dmPhysics::CollisionObjectData data;
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_STATIC;
    data.m_Mass = 0.0f;
    data.m_UserData = &vo_a;
    data.m_Group = 0xffff;
    data.m_Mask = 0xffff;

    const float hull_vertices[] = {  // 1x1 around origo
                                    -0.5f, -0.5f,
                                     0.5f, -0.5f,
                                     0.5f,  0.5f,
                                    -0.5f,  0.5f,

                                     // 1x0.5 with top aligned to x-axis
                                    -0.5f, -0.5f,
                                     0.5f, -0.5f,
                                     0.5f,  0.0f,
                                    -0.5f,  0.0f };

    const dmPhysics::HullDesc hulls[] = { {0, 4}, {4, 4} };
    dmPhysics::HHullSet2D hull_set = dmPhysics::NewHullSet2D(TestFixture::m_Context, hull_vertices, 8, hulls, 2);
    dmPhysics::HCollisionShape2D grid_shape = dmPhysics::NewGridShape2D(TestFixture::m_Context, hull_set, Point3(0,0,0), cell_width, cell_height, rows, columns);
    typename TypeParam::CollisionObjectType grid_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &grid_shape, 1u);

    for (int32_t row = 0; row < rows; ++row)
    {
        for (int32_t col = 0; col < columns; ++col)
        {
            dmPhysics::SetGridShapeHull(grid_co, grid_shape, row, col, 0);
        }
    }

    VisualObject vo_b;
    vo_b.m_Position = Point3(1, 33.0f, 0.0f);
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_DYNAMIC;
    data.m_Mass = 1.0f;
    data.m_UserData = &vo_b;
    data.m_Group = 0xffff;
    data.m_Mask = 0xffff;
    typename TypeParam::CollisionShapeType shape = (*TestFixture::m_Test.m_NewSphereShapeFunc)(TestFixture::m_Context, 0.5f);
    typename TypeParam::CollisionObjectType dynamic_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &shape, 1u);

    for (uint32_t i = 0; i < 30; ++i)
    {
        (*TestFixture::m_Test.m_StepWorldFunc)(TestFixture::m_World, TestFixture::m_StepWorldContext);
    }

    ASSERT_NEAR(32.0f + 0.5f, vo_b.m_Position.getY(), 0.05f);

    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, grid_co);
    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, dynamic_co);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(shape);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(grid_shape);
    dmPhysics::DeleteHullSet2D(hull_set);
}

bool TriggerCollisionCallback(void* user_data_a, uint16_t group_a, void* user_data_b, uint16_t group_b, void* user_data)
{
    VisualObject* vo = (VisualObject*)user_data_a;
    if (vo->m_CollisionCount == 0)
        vo->m_FirstCollisionGroup = group_a;
    ++vo->m_CollisionCount;

    vo = (VisualObject*)user_data_b;
    if (vo->m_CollisionCount == 0)
        vo->m_FirstCollisionGroup = group_b;
    ++vo->m_CollisionCount;
    int* count = (int*)user_data;
    *count += 1;
    return true;
}

TYPED_TEST(PhysicsTest, GridShapeTrigger)
{
    /*
     * Simplified version of GridShapePolygon
     */
    int32_t rows = 1;
    int32_t columns = 2;
    int32_t cell_width = 16;
    int32_t cell_height = 16;

    VisualObject vo_a;
    vo_a.m_Position = Point3(0, 0, 0);
    dmPhysics::CollisionObjectData data;
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_STATIC;
    data.m_Mass = 0.0f;
    data.m_UserData = &vo_a;
    data.m_Group = 0xffff;
    data.m_Mask = 0xffff;

    const float hull_vertices[] = {  // 1x1 around origo
                                    -0.5f, -0.5f,
                                     0.5f, -0.5f,
                                     0.5f,  0.5f,
                                    -0.5f,  0.5f };

    const dmPhysics::HullDesc hulls[] = { {0, 4} };
    dmPhysics::HHullSet2D hull_set = dmPhysics::NewHullSet2D(TestFixture::m_Context, hull_vertices, 4, hulls, 1);
    dmPhysics::HCollisionShape2D grid_shape = dmPhysics::NewGridShape2D(TestFixture::m_Context, hull_set, Point3(0,0,0), cell_width, cell_height, rows, columns);
    typename TypeParam::CollisionObjectType grid_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &grid_shape, 1u);

    dmPhysics::SetGridShapeHull(grid_co, grid_shape, 0, 0, 0);
    dmPhysics::SetGridShapeHull(grid_co, grid_shape, 0, 1, 0);

    VisualObject vo_b;
    vo_b.m_Position = Point3(4.0f, 4.0f, 0.0f);
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_TRIGGER;
    data.m_Mass = 0.0f;
    data.m_UserData = &vo_b;
    data.m_Group = 0xffff;
    data.m_Mask = 0xffff;
    typename TypeParam::CollisionShapeType shape = (*TestFixture::m_Test.m_NewBoxShapeFunc)(TestFixture::m_Context, Vector3(0.1f, 0.1f, 0.1f));
    typename TypeParam::CollisionObjectType dynamic_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &shape, 1u);

    TestFixture::m_StepWorldContext.m_CollisionCallback = TriggerCollisionCallback;
    for (int i = 0; i < 500; ++i)
    {
        (*TestFixture::m_Test.m_StepWorldFunc)(TestFixture::m_World, TestFixture::m_StepWorldContext);
        ASSERT_EQ(i+1, TestFixture::m_CollisionCount);
        ASSERT_EQ(0, TestFixture::m_ContactPointCount);
    }

    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, grid_co);
    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, dynamic_co);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(shape);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(grid_shape);
    dmPhysics::DeleteHullSet2D(hull_set);
}

struct CrackUserData
{
    CrackUserData()
    {
        memset(this, 0, sizeof(CrackUserData));
    }

    VisualObject* m_Target;
    Vector3 m_Normal;
    float m_Distance;
    uint32_t m_Count;
};

bool CrackContactPointCallback(const dmPhysics::ContactPoint& contact_point, void* user_data)
{
    CrackUserData* data = (CrackUserData*)user_data;
    data->m_Normal = contact_point.m_Normal;
    data->m_Distance = contact_point.m_Distance;
    ++data->m_Count;
    if (data->m_Target == contact_point.m_UserDataA)
    {
        data->m_Normal = -data->m_Normal;
    }
    return true;
}

// Tests colliding a thin box between two cells of a grid shape, due to a bug that disregards such collisions
TYPED_TEST(PhysicsTest, GridShapeCrack)
{
    /*
     * Simplified version of GridShapePolygon
     */
    int32_t rows = 1;
    int32_t columns = 2;
    int32_t cell_width = 16;
    int32_t cell_height = 16;

    VisualObject vo_a;
    vo_a.m_Position = Point3(0, 0, 0);
    dmPhysics::CollisionObjectData data;
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_STATIC;
    data.m_Mass = 0.0f;
    data.m_UserData = &vo_a;
    data.m_Group = 0xffff;
    data.m_Mask = 0xffff;

    const float hull_vertices[] = {-0.5f, -0.5f,
                                    0.5f, -0.5f,
                                    0.5f,  0.5f,
                                   -0.5f,  0.5f};

    const dmPhysics::HullDesc hulls[] = { {0, 4}, {0, 4} };
    dmPhysics::HHullSet2D hull_set = dmPhysics::NewHullSet2D(TestFixture::m_Context, hull_vertices, 4, hulls, 2);
    dmPhysics::HCollisionShape2D grid_shape = dmPhysics::NewGridShape2D(TestFixture::m_Context, hull_set, Point3(0,0,0), cell_width, cell_height, rows, columns);
    typename TypeParam::CollisionObjectType grid_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &grid_shape, 1u);

    for (int32_t row = 0; row < rows; ++row)
    {
        for (int32_t col = 0; col < columns; ++col)
        {
            dmPhysics::SetGridShapeHull(grid_co, grid_shape, row, col, 0);
        }
    }

    VisualObject vo_b;
    vo_b.m_Position = Point3(0.0f, 12.0f, 0.0f);
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_KINEMATIC;
    data.m_Mass = 0.0f;
    data.m_UserData = &vo_b;
    data.m_Group = 0xffff;
    data.m_Mask = 0xffff;
    typename TypeParam::CollisionShapeType shape = (*TestFixture::m_Test.m_NewBoxShapeFunc)(TestFixture::m_Context, Vector3(2.0f, 8.0f, 8.0f));
    typename TypeParam::CollisionObjectType dynamic_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &shape, 1u);

    CrackUserData crack_data;
    crack_data.m_Target = &vo_b;
    TestFixture::m_StepWorldContext.m_ContactPointCallback = CrackContactPointCallback;
    TestFixture::m_StepWorldContext.m_ContactPointUserData = &crack_data;
    (*TestFixture::m_Test.m_StepWorldFunc)(TestFixture::m_World, TestFixture::m_StepWorldContext);

    float eps = 0.000001f;
    ASSERT_EQ(4u, crack_data.m_Count);
    ASSERT_EQ(0.0f, crack_data.m_Normal.getX());
    ASSERT_EQ(1.0f, crack_data.m_Normal.getY());
    ASSERT_EQ(0.0f, crack_data.m_Normal.getZ());
    ASSERT_NEAR(4.0f, crack_data.m_Distance, eps);
    ASSERT_EQ(2, vo_a.m_CollisionCount);
    ASSERT_EQ(2, vo_b.m_CollisionCount);

    vo_b.m_Position = Point3(0.0f, 8.1f, 0.0f);
    (*TestFixture::m_Test.m_StepWorldFunc)(TestFixture::m_World, TestFixture::m_StepWorldContext);
    ASSERT_EQ(0.0f, crack_data.m_Normal.getX());
    ASSERT_EQ(1.0f, crack_data.m_Normal.getY());
    ASSERT_EQ(0.0f, crack_data.m_Normal.getZ());
    vo_b.m_Position = Point3(0.0f, 7.9f, 0.0f);
    (*TestFixture::m_Test.m_StepWorldFunc)(TestFixture::m_World, TestFixture::m_StepWorldContext);
    ASSERT_EQ(0.0f, crack_data.m_Normal.getX());
    ASSERT_EQ(-1.0f, crack_data.m_Normal.getY());
    ASSERT_EQ(0.0f, crack_data.m_Normal.getZ());

    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, grid_co);
    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, dynamic_co);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(shape);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(grid_shape);
    dmPhysics::DeleteHullSet2D(hull_set);
}

// Tests colliding a thin box at the corner of a cell of a grid shape
TYPED_TEST(PhysicsTest, GridShapeCorner)
{
    /*
     * Simplified version of GridShapePolygon
     */
    int32_t rows = 2;
    int32_t columns = 1;
    int32_t cell_width = 16;
    int32_t cell_height = 16;

    VisualObject vo_a;
    vo_a.m_Position = Point3(0, 0, 0);
    dmPhysics::CollisionObjectData data;
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_STATIC;
    data.m_Mass = 0.0f;
    data.m_UserData = &vo_a;
    data.m_Group = 0xffff;
    data.m_Mask = 0xffff;

    const float hull_vertices[] = {-0.5f, -0.5f,
                                    0.5f, -0.5f,
                                    0.5f,  0.5f,
                                   -0.5f,  0.5f};

    const dmPhysics::HullDesc hulls[] = { {0, 4}, {0, 4} };
    dmPhysics::HHullSet2D hull_set = dmPhysics::NewHullSet2D(TestFixture::m_Context, hull_vertices, 4, hulls, 2);
    dmPhysics::HCollisionShape2D grid_shape = dmPhysics::NewGridShape2D(TestFixture::m_Context, hull_set, Point3(0,0,0), cell_width, cell_height, rows, columns);
    typename TypeParam::CollisionObjectType grid_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &grid_shape, 1u);

    for (int32_t row = 0; row < rows; ++row)
    {
        for (int32_t col = 0; col < columns; ++col)
        {
            dmPhysics::SetGridShapeHull(grid_co, grid_shape, row, col, 0);
        }
    }

    VisualObject vo_b;
    vo_b.m_Position = Point3(-10.0f, 24.0f, 0.0f) + Vector3(0.15f, -0.1f, 0.0);
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_KINEMATIC;
    data.m_Mass = 0.0f;
    data.m_UserData = &vo_b;
    data.m_Group = 0xffff;
    data.m_Mask = 0xffff;
    typename TypeParam::CollisionShapeType shape = (*TestFixture::m_Test.m_NewBoxShapeFunc)(TestFixture::m_Context, Vector3(2.0f, 8.0f, 8.0f));
    typename TypeParam::CollisionObjectType dynamic_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &shape, 1u);

    CrackUserData crack_data;
    crack_data.m_Target = &vo_b;
    TestFixture::m_StepWorldContext.m_ContactPointCallback = CrackContactPointCallback;
    TestFixture::m_StepWorldContext.m_ContactPointUserData = &crack_data;
    (*TestFixture::m_Test.m_StepWorldFunc)(TestFixture::m_World, TestFixture::m_StepWorldContext);

    float eps = 0.000001f;
    ASSERT_EQ(2u, crack_data.m_Count);
    ASSERT_EQ(0.0f, crack_data.m_Normal.getX());
    ASSERT_EQ(1.0f, crack_data.m_Normal.getY());
    ASSERT_EQ(0.0f, crack_data.m_Normal.getZ());
    ASSERT_NEAR(0.1f, crack_data.m_Distance, eps);
    ASSERT_EQ(1, vo_a.m_CollisionCount);
    ASSERT_EQ(1, vo_b.m_CollisionCount);

    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, grid_co);
    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, dynamic_co);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(shape);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(grid_shape);
    dmPhysics::DeleteHullSet2D(hull_set);
}

// Tests colliding a sphere against a grid shape (edge) to verify collision distance
TYPED_TEST(PhysicsTest, GridShapeSphereDistance)
{
    /*
     * Simplified version of GridShapePolygon
     */
    int32_t rows = 1;
    int32_t columns = 3;
    int32_t cell_width = 16;
    int32_t cell_height = 16;

    VisualObject vo_a;
    vo_a.m_Position = Point3(0, 0, 0);
    dmPhysics::CollisionObjectData data;
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_STATIC;
    data.m_Mass = 0.0f;
    data.m_UserData = &vo_a;
    data.m_Group = 0xffff;
    data.m_Mask = 0xffff;

    const float hull_vertices[] = {-0.5f, -0.5f,
                                    0.5f, -0.5f,
                                    0.5f,  0.5f,
                                   -0.5f,  0.5f};

    const dmPhysics::HullDesc hulls[] = { {0, 4}, {0, 4}, {0, 4} };
    dmPhysics::HHullSet2D hull_set = dmPhysics::NewHullSet2D(TestFixture::m_Context, hull_vertices, 4, hulls, 3);
    dmPhysics::HCollisionShape2D grid_shape = dmPhysics::NewGridShape2D(TestFixture::m_Context, hull_set, Point3(0,0,0), cell_width, cell_height, rows, columns);
    typename TypeParam::CollisionObjectType grid_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &grid_shape, 1u);

    for (int32_t row = 0; row < rows; ++row)
    {
        for (int32_t col = 0; col < columns; ++col)
        {
            dmPhysics::SetGridShapeHull(grid_co, grid_shape, row, col, 0);
        }
    }

    VisualObject vo_b;
    vo_b.m_Position = Point3(0.0f, 14.0f, 0.0f);
    data.m_Type = dmPhysics::COLLISION_OBJECT_TYPE_KINEMATIC;
    data.m_Mass = 0.0f;
    data.m_UserData = &vo_b;
    data.m_Group = 0xffff;
    data.m_Mask = 0xffff;
    typename TypeParam::CollisionShapeType shape = (*TestFixture::m_Test.m_NewSphereShapeFunc)(TestFixture::m_Context, 8.0f);
    typename TypeParam::CollisionObjectType dynamic_co = (*TestFixture::m_Test.m_NewCollisionObjectFunc)(TestFixture::m_World, data, &shape, 1u);

    CrackUserData crack_data;
    crack_data.m_Target = &vo_b;
    TestFixture::m_StepWorldContext.m_ContactPointCallback = CrackContactPointCallback;
    TestFixture::m_StepWorldContext.m_ContactPointUserData = &crack_data;
    (*TestFixture::m_Test.m_StepWorldFunc)(TestFixture::m_World, TestFixture::m_StepWorldContext);

    float eps = 0.000001f;
    ASSERT_EQ(1u, crack_data.m_Count);
    ASSERT_EQ(0.0f, crack_data.m_Normal.getX());
    ASSERT_EQ(1.0f, crack_data.m_Normal.getY());
    ASSERT_EQ(0.0f, crack_data.m_Normal.getZ());
    ASSERT_NEAR(2.0f, crack_data.m_Distance, eps);
    ASSERT_EQ(1, vo_a.m_CollisionCount);
    ASSERT_EQ(1, vo_b.m_CollisionCount);

    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, grid_co);
    (*TestFixture::m_Test.m_DeleteCollisionObjectFunc)(TestFixture::m_World, dynamic_co);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(shape);
    (*TestFixture::m_Test.m_DeleteCollisionShapeFunc)(grid_shape);
    dmPhysics::DeleteHullSet2D(hull_set);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
