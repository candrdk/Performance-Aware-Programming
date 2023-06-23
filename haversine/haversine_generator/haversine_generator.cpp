#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static double Square(double A)
{
    double Result = (A * A);
    return Result;
}

static double RadiansFromDegrees(double Degrees)
{
    double Result = 0.01745329251994329577 * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static double ReferenceHaversine(double X0, double Y0, double X1, double Y1, double EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */

    double lat1 = Y0;
    double lat2 = Y1;
    double lon1 = X0;
    double lon2 = X1;

    double dLat = RadiansFromDegrees(lat2 - lat1);
    double dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);

    double a = Square(sin(dLat / 2.0)) + cos(lat1) * cos(lat2) * Square(sin(dLon / 2));
    double c = 2.0 * asin(sqrt(a));

    double Result = EarthRadius * c;

    return Result;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: haversine_generator [seed] [count] [1: json, 0: sum]");
        return 0;
    }

    srand(atoi(argv[1]));
    int count = atoi(argv[2]);

    if (count <= 0) {
        printf("Usage: [count] must be >= 0.");
        return 0;
    }
    
    double sum = 0.0;

    bool printJSON = (argc == 4) && (argv[3][0] == '0');

    if (printJSON) {
        printf("{\"pairs\":[\n");
    }

    for (int i = 0; i < count; i++) {
        // https://mathworld.wolfram.com/SpherePointPicking.html
        // Let u, v be random variates on (0, 1), then
        //      theta = 2*pi*u,    phi = acos(2v - 1)
        // gives the spherical coordinates for a set of points
        // which are uniformly distributed over S^2.
        double x0 = 2.0 * 3.1415 * rand() / double(RAND_MAX);
        double x1 = 2.0 * 3.1415 * rand() / double(RAND_MAX);
        double y0 = acos((2.0 * rand() / double(RAND_MAX)) - 1.0);
        double y1 = acos((2.0 * rand() / double(RAND_MAX)) - 1.0);

        // (0, 2*pi) -> (-180; 180)
        x0 = 57.2958 * (x0 - 3.1415);
        x1 = 57.2958 * (x1 - 3.1415);

        // (0, pi) -> (-90; 90)
        y0 = 57.2958 * (y0 - 0.5 * 3.1415);
        y1 = 57.2958 * (y1 - 0.5 * 3.1415);

        if (printJSON) {
            printf("{\"x0\":%f, \"y0\":%f, \"x1\":%f, \"y1\":%f}%c\n", x0, y0, x1, y1, i == count - 1 ? ' ' : ',');
        }

        sum += ReferenceHaversine(x0, y0, x1, y1, 6372.8);
    }

    if (printJSON) {
        printf("]}");
    }

    if (!printJSON) {
        printf("\nsum: %f\n", sum / count);
    }

    return 0;
}