#include <fstream>
#include <iostream>
#include <cmath>

struct Vec3 {
    double x, y, z;

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
};

    double hashDirecao(const Vec3& direcao) {
    double valor = std::sin(direcao.x * 127.1 + direcao.y * 311.7 + direcao.z * 74.7) * 43758.5453;
    return valor - std::floor(valor);
}

int main() {
    const int largura = 400;
    const int altura = 300;

    const double distanciaCamera = 15.0;   // câmera afastada 15 unidades do buraco negro
    const double campoDeVisao = 1.5;       // "zoom" — quanto maior, mais aberto o ângulo de visão

    const double raioSchwarzschild = 1.0;  // "tamanho" do buraco negro, nossa unidade de referência
    const double discoRaioInterno = 2.2 * raioSchwarzschild;
    const double discoRaioExterno = 8.0 * raioSchwarzschild;
    const double passoAngular = 0.01;      // o quão fino é cada passo da simulação
    const int maxPassos = 4000;            // limite de segurança pra não rodar pra sempre

    Vec3 posicaoCamera = { 0, distanciaCamera * 0.3, distanciaCamera * 0.95 };

    std::ofstream arquivo("imagem.ppm");
    arquivo << "P3\n" << largura << " " << altura << "\n255\n";

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            double u_tela = (2.0 * x / largura - 1.0) * (double(largura) / altura);
            double v_tela = 1.0 - 2.0 * y / altura;

            Vec3 direcaoRaio = { u_tela * campoDeVisao, v_tela * campoDeVisao, -1.0 };
            direcaoRaio = direcaoRaio.normalizado();
            Vec3 direcaoOriginal = direcaoRaio;

            double r0 = posicaoCamera.comprimento();
            double paramImpacto = posicaoCamera.produtoVetorial(direcaoRaio).comprimento();

            double u = 1.0 / r0;
            double uLinha = -(posicaoCamera.produtoEscalar(direcaoRaio)) / (r0 * paramImpacto);

            bool capturado = false;
            bool atingiuDisco = false;

            for (int passo = 0; passo < maxPassos; passo++) {
                auto aceleracao = [raioSchwarzschild](double u) {
                    return -u + 1.5 * raioSchwarzschild * u * u;
                };

                double k1_u = uLinha;
                double k1_up = aceleracao(u);

                double k2_u = uLinha + 0.5 * passoAngular * k1_up;
                double k2_up = aceleracao(u + 0.5 * passoAngular * k1_u);

                double k3_u = uLinha + 0.5 * passoAngular * k2_up;
                double k3_up = aceleracao(u + 0.5 * passoAngular * k2_u);

                double k4_u = uLinha + passoAngular * k3_up;
                double k4_up = aceleracao(u + passoAngular * k3_u);

                Vec3 e1 = posicaoCamera.normalizado();
                Vec3 e2 = (direcaoRaio - e1 * direcaoRaio.produtoEscalar(e1)).normalizado();
                u = u + (passoAngular / 6.0) * (k1_u + 2*k2_u + 2*k3_u + k4_u);
                uLinha = uLinha + (passoAngular / 6.0) * (k1_up + 2*k2_up + 2*k3_up + k4_up);

                double phi = (passo + 1) * passoAngular;
                double raioAtual = 1.0 / u;
                Vec3 posicaoAtual = e1 * (raioAtual * std::cos(phi)) + e2 * (raioAtual * std::sin(phi));

                if (std::abs(posicaoAtual.y) < 0.15 && raioAtual > discoRaioInterno && raioAtual < discoRaioExterno) {
                    atingiuDisco = true;
                    break;
                }

                if (u > 1.0 / raioSchwarzschild) {
                    capturado = true;
                    break;
                }
                if (u < 1.0 / (3.0 * r0)) {
                    break;
                }
            }

            int r, g, b;
            if (capturado) {
                r = 0; g = 0; b = 0;
            } else if (atingiuDisco) {
                r = 255; g = 150; b = 60;
            } else {
                double chance = hashDirecao(direcaoOriginal);

                if (chance > 0.997) {
                    int brilho = 180 + int(chance * 75);
                    r = brilho; g = brilho; b = brilho;
                } else {
                    r = 5; g = 5; b = 15;
                }
            }

            arquivo << r << " " << g << " " << b << " ";
        }
        arquivo << "\n";
    }

    arquivo.close();
    std::cout << "Imagem gerada: imagem.ppm\n";

    return 0;
}