namespace {
    // Направления осей (положительные)
    const Vector3i AXIS[] = {
        Vector3i(1, 0, 0),
        Vector3i(0, 1, 0),
        Vector3i(0, 0, 1)
    };

    // Углы квадрата для каждой оси
    const Vector3i QUAD_POINTS[3][4] = {
        // Ось X
        {
            Vector3i(0, 0, -1),
            Vector3i(0, -1, -1),
            Vector3i(0, -1, 0),
            Vector3i(0, 0, 0)
        },
        // Ось Y
        {
            Vector3i(0, 0, -1),
            Vector3i(0, 0, 0),
            Vector3i(-1, 0, 0),
            Vector3i(-1, 0, -1)
        },
        // Ось Z
        {
            Vector3i(0, 0, 0),
            Vector3i(0, -1, 0),
            Vector3i(-1, -1, 0),
            Vector3i(-1, 0, 0)
        }
    };

    // 12 рёбер куба (пары точек)
    const Vector3i EDGE_OFFSETS[12][2] = {
        // Рёбра на min Z
        { Vector3i(0, 0, 0), Vector3i(1, 0, 0) },
        { Vector3i(1, 0, 0), Vector3i(1, 1, 0) },
        { Vector3i(1, 1, 0), Vector3i(0, 1, 0) },
        { Vector3i(0, 1, 0), Vector3i(0, 0, 0) },
        // Рёбра на max Z
        { Vector3i(0, 0, 1), Vector3i(1, 0, 1) },
        { Vector3i(1, 0, 1), Vector3i(1, 1, 1) },
        { Vector3i(1, 1, 1), Vector3i(0, 1, 1) },
        { Vector3i(0, 1, 1), Vector3i(0, 0, 1) },
        // Рёбра, соединяющие min Z и max Z
        { Vector3i(0, 0, 0), Vector3i(0, 0, 1) },
        { Vector3i(1, 0, 0), Vector3i(1, 0, 1) },
        { Vector3i(1, 1, 0), Vector3i(1, 1, 1) },
        { Vector3i(0, 1, 0), Vector3i(0, 1, 1) }
    };
}