#include <fstream>
#include <iostream>
#include <cmath>

struct Vec3 {
    double x, y, z;
    double produtoEscalar(const Vec3& outro) const {
        return x * outro.x + y * outro.y + z * outro.z;
    }

    Vec3 produtoVetorial(const Vec3& outro) const {
        return {
            y * outro.z - z * outro.y,
            z * outro.x - x * outro.z,
            x * outro.y - y * outro.x
        };
    }

    Vec3 operator+(const Vec3& outro) const {
        return { x + outro.x, y + outro.y, z + outro.z };
    }

    Vec3 operator-(const Vec3& outro) const {
        return { x - outro.x, y - outro.y, z - outro.z };
    }

    Vec3 operator*(double escalar) const {
        return { x * escalar, y * escalar, z * escalar };
    }

    double comprimento() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 normalizado() const {
        double c = comprimento();
        return { x / c, y / c, z / c };
    }
};

int main() {
    const int largura = 400;
    const int altura = 300;

    const double distanciaCamera = 15.0;  // câmera afastada 15 unidades do buraco negro
    const double campoDeVisao = 1.5;      // "zoom" — quanto maior, mais aberto o ângulo de visão

    Vec3 posicaoCamera = { 0, 0, distanciaCamera };

    std::ofstream arquivo("imagem.ppm");
    arquivo << "P3\n" << largura << " " << altura << "\n255\n";

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            // Converte a posição do pixel (x, y) pra coordenadas de -1 a 1
            double u = (2.0 * x / largura - 1.0) * (double(largura) / altura);
            double v = 1.0 - 2.0 * y / altura;

            // Direção do raio saindo da câmera, olhando em direção ao buraco negro (-z)
            Vec3 direcaoRaio = { u * campoDeVisao, v * campoDeVisao, -1.0 };
            direcaoRaio = direcaoRaio.normalizado();

                        double r0 = posicaoCamera.comprimento();
            double paramImpacto = posicaoCamera.produtoVetorial(direcaoRaio).comprimento();

            // Teste: só imprime os valores do pixel bem no centro da imagem
            if (x == largura / 2 && y == altura / 2) {
                std::cout << "Pixel central -> r0: " << r0 << ", b: " << paramImpacto << "\n";
            }

            // Por enquanto, só pra visualizar: pinta baseado na direção do raio
            int r = int((direcaoRaio.x + 1.0) * 127);
            int g = int((direcaoRaio.y + 1.0) * 127);
            int b = int((-direcaoRaio.z) * 255);

            arquivo << r << " " << g << " " << b << " ";
        }
        arquivo << "\n";
    }

    arquivo.close();
    std::cout << "Imagem gerada: imagem.ppm\n";

    return 0;
}
