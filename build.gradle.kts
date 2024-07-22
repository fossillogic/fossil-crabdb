plugins {
    kotlin("jvm") version "1.8.0"
    application
}

group = "com.fossillogic.bluecrabdb"
version = "1.0-SNAPSHOT"

repositories {
    mavenCentral()
}

dependencies {
    implementation(kotlin("stdlib"))
    testImplementation(kotlin("test"))
    testImplementation("org.jetbrains.kotlin:kotlin-test-junit:1.8.0")
}

tasks.test {
    useJUnitPlatform()
}

sourceSets {
    main {
        kotlin {
            srcDirs("code/logic/fossil")
        }
    }
    test {
        kotlin {
            srcDirs("code/tests")
        }
    }
}

application {
    mainClass.set("com.fossillogic.bluecrabdb.MainKt")
}

tasks.withType<org.jetbrains.kotlin.gradle.tasks.KotlinCompile> {
    kotlinOptions {
        jvmTarget = "1.8"
    }
}
