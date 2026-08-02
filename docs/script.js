/* ==========================================
   RTX Launcher — JavaScript
   Particles, Animations, Interactions
   ========================================== */

document.addEventListener('DOMContentLoaded', () => {
    initParticles();
    initScrollAnimations();
    initNavbar();
    initMobileMenu();
    initComparisonSlider();
    initFAQ();
    initCounters();
    initOSDetection();
});

/* ==========================================
   OS DETECTION — Windows-only download
   ========================================== */
function initOSDetection() {
    const ua = navigator.userAgent || navigator.platform || '';
    const isWindows = /Win/i.test(ua);

    const downloadBtns = document.querySelectorAll('[data-download]');

    if (!isWindows) {
        downloadBtns.forEach(btn => {
            // Remove href so it does nothing
            btn.removeAttribute('href');
            btn.removeAttribute('target');
            btn.removeAttribute('rel');

            // Remove existing classes and add disabled style
            btn.classList.remove('btn-primary', 'btn-outline');
            btn.classList.add('btn-disabled');

            // Replace inner content (keep SVG icon but swap to a Windows icon + text)
            btn.innerHTML = `
                <svg width="18" height="18" viewBox="0 0 24 24" fill="currentColor">
                    <path d="M0 3.449L9.75 2.1v9.451H0m10.949-9.602L24 0v11.4H10.949M0 12.6h9.75v9.451L0 20.699M10.949 12.6H24V24l-12.9-1.801"/>
                </svg>
                Доступно только на Windows
            `;

            // Prevent any click action
            btn.addEventListener('click', (e) => {
                e.preventDefault();
                e.stopPropagation();
            });

            // Set cursor and style via attribute
            btn.style.cursor = 'not-allowed';
        });
    }
}

/* ==========================================
   PARTICLE SYSTEM
   ========================================== */
function initParticles() {
    const canvas = document.getElementById('particles-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    let width, height;
    let particles = [];
    let mouse = { x: -1000, y: -1000 };
    const PARTICLE_COUNT = 80;
    const CONNECTION_DISTANCE = 140;
    const MOUSE_RADIUS = 180;

    function resize() {
        width = canvas.width = window.innerWidth;
        height = canvas.height = window.innerHeight;
    }

    class Particle {
        constructor() {
            this.reset();
        }

        reset() {
            this.x = Math.random() * width;
            this.y = Math.random() * height;
            this.size = Math.random() * 1.8 + 0.5;
            this.speedX = (Math.random() - 0.5) * 0.4;
            this.speedY = (Math.random() - 0.5) * 0.4;
            this.opacity = Math.random() * 0.5 + 0.1;
            // Color: mix between green and cyan
            const t = Math.random();
            this.r = 0;
            this.g = Math.floor(255 * (1 - t) + 212 * t);
            this.b = Math.floor(136 * (1 - t) + 255 * t);
        }

        update() {
            this.x += this.speedX;
            this.y += this.speedY;

            // Mouse interaction
            const dx = mouse.x - this.x;
            const dy = mouse.y - this.y;
            const dist = Math.sqrt(dx * dx + dy * dy);
            if (dist < MOUSE_RADIUS) {
                const force = (MOUSE_RADIUS - dist) / MOUSE_RADIUS * 0.015;
                this.speedX -= dx * force;
                this.speedY -= dy * force;
            }

            // Damping
            this.speedX *= 0.999;
            this.speedY *= 0.999;

            // Wrap around edges
            if (this.x < -10) this.x = width + 10;
            if (this.x > width + 10) this.x = -10;
            if (this.y < -10) this.y = height + 10;
            if (this.y > height + 10) this.y = -10;
        }

        draw() {
            ctx.beginPath();
            ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
            ctx.fillStyle = `rgba(${this.r}, ${this.g}, ${this.b}, ${this.opacity})`;
            ctx.fill();
        }
    }

    function createParticles() {
        particles = [];
        for (let i = 0; i < PARTICLE_COUNT; i++) {
            particles.push(new Particle());
        }
    }

    function drawConnections() {
        for (let i = 0; i < particles.length; i++) {
            for (let j = i + 1; j < particles.length; j++) {
                const dx = particles[i].x - particles[j].x;
                const dy = particles[i].y - particles[j].y;
                const dist = Math.sqrt(dx * dx + dy * dy);

                if (dist < CONNECTION_DISTANCE) {
                    const opacity = (1 - dist / CONNECTION_DISTANCE) * 0.12;
                    ctx.beginPath();
                    ctx.moveTo(particles[i].x, particles[i].y);
                    ctx.lineTo(particles[j].x, particles[j].y);
                    ctx.strokeStyle = `rgba(0, 255, 136, ${opacity})`;
                    ctx.lineWidth = 0.5;
                    ctx.stroke();
                }
            }
        }
    }

    function animate() {
        ctx.clearRect(0, 0, width, height);

        particles.forEach(p => {
            p.update();
            p.draw();
        });

        drawConnections();
        requestAnimationFrame(animate);
    }

    // Events
    window.addEventListener('resize', () => {
        resize();
    });

    window.addEventListener('mousemove', (e) => {
        mouse.x = e.clientX;
        mouse.y = e.clientY;
    });

    window.addEventListener('mouseleave', () => {
        mouse.x = -1000;
        mouse.y = -1000;
    });

    resize();
    createParticles();
    animate();
}

/* ==========================================
   SCROLL ANIMATIONS (Intersection Observer)
   ========================================== */
function initScrollAnimations() {
    const animatedElements = document.querySelectorAll('[data-animate]');

    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                const delay = parseInt(entry.target.dataset.delay) || 0;
                setTimeout(() => {
                    entry.target.classList.add('animated');
                }, delay);
                observer.unobserve(entry.target);
            }
        });
    }, {
        threshold: 0.1,
        rootMargin: '0px 0px -40px 0px'
    });

    animatedElements.forEach(el => observer.observe(el));
}

/* ==========================================
   NAVBAR SCROLL EFFECT
   ========================================== */
function initNavbar() {
    const navbar = document.getElementById('navbar');
    if (!navbar) return;

    let lastScroll = 0;
    let ticking = false;

    function updateNavbar() {
        const scrollY = window.scrollY;

        if (scrollY > 50) {
            navbar.classList.add('scrolled');
        } else {
            navbar.classList.remove('scrolled');
        }

        lastScroll = scrollY;
        ticking = false;
    }

    window.addEventListener('scroll', () => {
        if (!ticking) {
            requestAnimationFrame(updateNavbar);
            ticking = true;
        }
    }, { passive: true });

    // Smooth scroll for nav links
    document.querySelectorAll('a[href^="#"]').forEach(link => {
        link.addEventListener('click', (e) => {
            const target = document.querySelector(link.getAttribute('href'));
            if (target) {
                e.preventDefault();
                const offset = navbar.offsetHeight + 20;
                const top = target.getBoundingClientRect().top + window.scrollY - offset;
                window.scrollTo({ top, behavior: 'smooth' });

                // Close mobile menu if open
                const navLinks = document.getElementById('nav-links');
                const navToggle = document.getElementById('nav-toggle');
                if (navLinks && navLinks.classList.contains('open')) {
                    navLinks.classList.remove('open');
                    navToggle.classList.remove('active');
                    document.body.style.overflow = '';
                }
            }
        });
    });
}

/* ==========================================
   MOBILE MENU
   ========================================== */
function initMobileMenu() {
    const toggle = document.getElementById('nav-toggle');
    const navLinks = document.getElementById('nav-links');
    if (!toggle || !navLinks) return;

    toggle.addEventListener('click', () => {
        const isOpen = navLinks.classList.toggle('open');
        toggle.classList.toggle('active');
        document.body.style.overflow = isOpen ? 'hidden' : '';
    });
}

/* ==========================================
   COMPARISON SLIDER
   ========================================== */
function initComparisonSlider() {
    const slider = document.getElementById('comparison-slider');
    const handle = document.getElementById('comparison-handle');
    if (!slider || !handle) return;

    const afterSide = slider.querySelector('.comparison-after');
    let isDragging = false;

    function setPosition(x) {
        const rect = slider.getBoundingClientRect();
        let percent = ((x - rect.left) / rect.width) * 100;
        percent = Math.max(5, Math.min(95, percent));

        afterSide.style.clipPath = `inset(0 0 0 ${percent}%)`;
        handle.style.left = `${percent}%`;
    }

    function onStart(e) {
        isDragging = true;
        slider.style.cursor = 'grabbing';
        const clientX = e.touches ? e.touches[0].clientX : e.clientX;
        setPosition(clientX);
    }

    function onMove(e) {
        if (!isDragging) return;
        e.preventDefault();
        const clientX = e.touches ? e.touches[0].clientX : e.clientX;
        setPosition(clientX);
    }

    function onEnd() {
        isDragging = false;
        slider.style.cursor = 'ew-resize';
    }

    slider.addEventListener('mousedown', onStart);
    slider.addEventListener('touchstart', onStart, { passive: true });

    window.addEventListener('mousemove', onMove);
    window.addEventListener('touchmove', onMove, { passive: false });

    window.addEventListener('mouseup', onEnd);
    window.addEventListener('touchend', onEnd);

    // Animated intro: slide from 80% to 50%
    let introProgress = 0;
    const introObserver = new IntersectionObserver((entries) => {
        if (entries[0].isIntersecting) {
            animateSliderIntro();
            introObserver.unobserve(slider);
        }
    }, { threshold: 0.3 });

    introObserver.observe(slider);

    function animateSliderIntro() {
        const start = 80;
        const end = 50;
        const duration = 1200;
        const startTime = performance.now();

        function step(currentTime) {
            const elapsed = currentTime - startTime;
            const progress = Math.min(elapsed / duration, 1);
            // Ease out cubic
            const eased = 1 - Math.pow(1 - progress, 3);
            const current = start + (end - start) * eased;

            afterSide.style.clipPath = `inset(0 0 0 ${current}%)`;
            handle.style.left = `${current}%`;

            if (progress < 1) {
                requestAnimationFrame(step);
            }
        }

        requestAnimationFrame(step);
    }
}

/* ==========================================
   FAQ ACCORDION
   ========================================== */
function initFAQ() {
    const faqItems = document.querySelectorAll('.faq-item');

    faqItems.forEach(item => {
        const question = item.querySelector('.faq-question');

        question.addEventListener('click', () => {
            const isActive = item.classList.contains('active');

            // Close all
            faqItems.forEach(other => {
                other.classList.remove('active');
            });

            // Toggle current
            if (!isActive) {
                item.classList.add('active');
            }
        });
    });
}

/* ==========================================
   ANIMATED COUNTERS
   ========================================== */
function initCounters() {
    const counters = document.querySelectorAll('.stat-number[data-count]');
    if (!counters.length) return;

    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                animateCounter(entry.target);
                observer.unobserve(entry.target);
            }
        });
    }, { threshold: 0.5 });

    counters.forEach(counter => observer.observe(counter));

    function animateCounter(el) {
        const target = parseInt(el.dataset.count);
        const duration = 2000;
        const startTime = performance.now();

        function step(currentTime) {
            const elapsed = currentTime - startTime;
            const progress = Math.min(elapsed / duration, 1);
            // Ease out expo
            const eased = progress === 1 ? 1 : 1 - Math.pow(2, -10 * progress);
            const current = Math.floor(target * eased);

            el.textContent = current;

            if (progress < 1) {
                requestAnimationFrame(step);
            } else {
                el.textContent = target;
            }
        }

        requestAnimationFrame(step);
    }
}
