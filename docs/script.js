/* ================================================
   RTX Launcher — script.js
   Scroll animations · Navbar · Comparison slider
   FAQ · Counters · OS detection
   ================================================ */

document.addEventListener('DOMContentLoaded', () => {
    initScrollAnimations();
    initNavbar();
    initMobileMenu();
    initComparisonSlider();
    initFAQ();
    initCounters();
    initOSDetection();
    initCardGlow();
});

/* ================================================
   OS DETECTION — Windows-only download
   ================================================ */
function initOSDetection() {
    const ua = navigator.userAgent || navigator.platform || '';
    const isWindows = /Win/i.test(ua);
    const downloadBtns = document.querySelectorAll('[data-download]');

    if (!isWindows) {
        downloadBtns.forEach(btn => {
            btn.removeAttribute('href');
            btn.removeAttribute('target');
            btn.removeAttribute('rel');
            btn.classList.remove('btn-primary', 'btn-outline');
            btn.classList.add('btn-disabled');
            btn.innerHTML = `
                <svg width="16" height="16" viewBox="0 0 24 24" fill="currentColor">
                    <path d="M0 3.449L9.75 2.1v9.451H0m10.949-9.602L24 0v11.4H10.949M0 12.6h9.75v9.451L0 20.699M10.949 12.6H24V24l-12.9-1.801"/>
                </svg>
                Доступно только на Windows
            `;
            btn.style.cursor = 'not-allowed';
            btn.addEventListener('click', e => { e.preventDefault(); e.stopPropagation(); });
        });
    }
}

/* ================================================
   NAVBAR — scroll-aware glass effect
   ================================================ */
function initNavbar() {
    const navbar = document.getElementById('navbar');
    if (!navbar) return;

    const update = () => {
        if (window.scrollY > 24) {
            navbar.classList.add('scrolled');
        } else {
            navbar.classList.remove('scrolled');
        }
    };

    window.addEventListener('scroll', update, { passive: true });
    update();
}

/* ================================================
   MOBILE MENU
   ================================================ */
function initMobileMenu() {
    const toggle = document.getElementById('nav-toggle');
    const links  = document.getElementById('nav-links');
    if (!toggle || !links) return;

    toggle.addEventListener('click', () => {
        links.classList.toggle('mobile-open');
    });

    // Close on link click
    links.querySelectorAll('.nav-link').forEach(link => {
        link.addEventListener('click', () => links.classList.remove('mobile-open'));
    });
}

/* ================================================
   SCROLL ANIMATIONS — IntersectionObserver
   ================================================ */
function initScrollAnimations() {
    const els = document.querySelectorAll('[data-animate]');
    if (!els.length) return;

    const observer = new IntersectionObserver(
        (entries) => {
            entries.forEach(entry => {
                if (entry.isIntersecting) {
                    entry.target.classList.add('visible');
                    observer.unobserve(entry.target);
                }
            });
        },
        { threshold: 0.12, rootMargin: '0px 0px -40px 0px' }
    );

    els.forEach(el => observer.observe(el));
}

/* ================================================
   COMPARISON SLIDER
   ================================================ */
function initComparisonSlider() {
    const slider = document.getElementById('comparison-slider');
    const handle = document.getElementById('comparison-handle');
    if (!slider || !handle) return;

    const after  = slider.querySelector('.comparison-after');

    let dragging = false;
    let startX   = 0;
    let startVal = 50; // percent
    let currentVal = 50;

    function setPosition(pct) {
        pct = Math.max(2, Math.min(98, pct));
        currentVal = pct;
        after.style.right = (100 - pct) + '%';
        handle.style.left = pct + '%';
    }

    function getPercent(e) {
        const rect = slider.getBoundingClientRect();
        const clientX = e.touches ? e.touches[0].clientX : e.clientX;
        return ((clientX - rect.left) / rect.width) * 100;
    }

    function onStart(e) {
        dragging = true;
        startX   = getPercent(e);
        startVal = currentVal;
        slider.style.cursor = 'col-resize';
        e.preventDefault();
    }

    function onMove(e) {
        if (!dragging) return;
        const pct = getPercent(e);
        setPosition(pct);
    }

    function onEnd() {
        dragging = false;
        slider.style.cursor = 'col-resize';
    }

    handle.addEventListener('mousedown',  onStart);
    handle.addEventListener('touchstart', onStart, { passive: false });
    slider.addEventListener('mousedown',  onStart);
    slider.addEventListener('touchstart', onStart, { passive: false });

    window.addEventListener('mousemove', onMove);
    window.addEventListener('touchmove', onMove, { passive: true });
    window.addEventListener('mouseup',   onEnd);
    window.addEventListener('touchend',  onEnd);

    // Init to 50%
    setPosition(50);
}

/* ================================================
   FAQ ACCORDION
   ================================================ */
function initFAQ() {
    const items = document.querySelectorAll('.faq-item');
    if (!items.length) return;

    items.forEach(item => {
        const btn    = item.querySelector('.faq-question');
        const answer = item.querySelector('.faq-answer');
        if (!btn || !answer) return;

        btn.addEventListener('click', () => {
            const isOpen = item.classList.contains('open');

            // Close all
            items.forEach(i => {
                i.classList.remove('open');
                const a = i.querySelector('.faq-answer');
                if (a) a.style.maxHeight = null;
            });

            // Open clicked (if it was closed)
            if (!isOpen) {
                item.classList.add('open');
                answer.style.maxHeight = answer.scrollHeight + 'px';
            }
        });
    });
}

/* ================================================
   COUNTER ANIMATION
   ================================================ */
function initCounters() {
    const counters = document.querySelectorAll('[data-count]');
    if (!counters.length) return;

    const observer = new IntersectionObserver(entries => {
        entries.forEach(entry => {
            if (!entry.isIntersecting) return;

            const el     = entry.target;
            const target = parseInt(el.dataset.count, 10);
            const duration = 1400;
            const start  = performance.now();

            function tick(now) {
                const elapsed = now - start;
                const progress = Math.min(elapsed / duration, 1);
                // ease-out-expo
                const eased = 1 - Math.pow(2, -10 * progress);
                el.textContent = Math.floor(eased * target);
                if (progress < 1) requestAnimationFrame(tick);
                else el.textContent = target;
            }

            requestAnimationFrame(tick);
            observer.unobserve(el);
        });
    }, { threshold: 0.5 });

    counters.forEach(c => observer.observe(c));
}

/* ================================================
   CARD FOLLOW-GLOW (subtle pointer tracking)
   ================================================ */
function initCardGlow() {
    const cards = document.querySelectorAll('.bento-card, .comp-feature, .req-card, .faq-item');

    cards.forEach(card => {
        card.addEventListener('mousemove', e => {
            const rect = card.getBoundingClientRect();
            const x = ((e.clientX - rect.left) / rect.width  * 100).toFixed(1);
            const y = ((e.clientY - rect.top)  / rect.height * 100).toFixed(1);
            card.style.setProperty('--mx', `${x}%`);
            card.style.setProperty('--my', `${y}%`);
            card.style.background = `radial-gradient(circle at ${x}% ${y}%, rgba(57,232,134,0.05) 0%, var(--glass-bg) 60%)`;
        });

        card.addEventListener('mouseleave', () => {
            card.style.background = '';
        });
    });
}
